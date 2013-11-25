// Copyright (c) 2004-2013 Sergey Lyubka
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _XOPEN_SOURCE 600  // For PATH_MAX on linux

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdarg.h>
#include <ctype.h>

#include <vector>
using std::vector;
#include <map>
using std::map;
#include <string>
using std::string;

#include "mongoose.h"

using namespace mongoose;

#include <sys/wait.h>
#include <unistd.h>
#define DIRSEP '/'
#define WINCDECL
#define abs_path(rel, abs, abs_size) realpath((rel), (abs))

#define MAX_OPTIONS 100
#define MAX_CONF_FILE_LINE_SIZE (8 * 1024)

static int exit_flag;
static char server_name[40];        // Set by init_server_name()
static char config_file[PATH_MAX];  // Set by process_command_line_arguments()
static struct mg_context *ctx;      // Set by start_mongoose()

#if !defined(CONFIG_FILE)
#define CONFIG_FILE "mongoose.conf"
#endif /* !CONFIG_FILE */

static void WINCDECL signal_handler(int sig_num) {
  // Reinstantiate signal handler
  signal(sig_num, signal_handler);

  // Do not do the trick with ignoring SIGCHLD, cause not all OSes (e.g. QNX)
  // reap zombies if SIGCHLD is ignored. On QNX, for example, waitpid()
  // fails if SIGCHLD is ignored, making system() non-functional.
  if (sig_num == SIGCHLD) {
    do {} while (waitpid(-1, &sig_num, WNOHANG) > 0);
  } else

  { exit_flag = sig_num; }
}

static void die(const char *fmt, ...) {
  va_list ap;
  char msg[200];

  va_start(ap, fmt);
  vsnprintf(msg, sizeof(msg), fmt, ap);
  va_end(ap);

#if defined(_WIN32)
  MessageBox(NULL, msg, "Error", MB_OK);
#else
  fprintf(stderr, "%s\n", msg);
#endif

  exit(EXIT_FAILURE);
}

static void show_usage_and_exit(void) {
  const char **names;
  int i;

  fprintf(stderr, "Mongoose version %s (c) Sergey Lyubka, built on %s\n",
          mg_version(), __DATE__);
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  mongoose -A <htpasswd_file> <realm> <user> <passwd>\n");
  fprintf(stderr, "  mongoose [config_file]\n");
  fprintf(stderr, "  mongoose [-option value ...]\n");
  fprintf(stderr, "\nOPTIONS:\n");

  names = mg_get_valid_option_names();
  for (i = 0; names[i] != NULL; i += 2) {
    fprintf(stderr, "  -%s %s\n",
            names[i], names[i + 1] == NULL ? "<empty>" : names[i + 1]);
  }
  exit(EXIT_FAILURE);
}

static void process_command_line_arguments(const vector<string>& args, map<string, string>& options) {
  char line[MAX_CONF_FILE_LINE_SIZE], opt[sizeof(line)], val[sizeof(line)];
  FILE *fp = NULL;
  size_t i, cmd_line_opts_start = 1, line_no = 0;

  // Should we use a config file ?
  if (args.size() >= 2 && args[1][0] != '-') {
    snprintf(config_file, sizeof(config_file), "%s", args[1].c_str());
    cmd_line_opts_start = 2;
  } else {
    size_t pos = args[0].rfind(DIRSEP);
    if (pos == string::npos) {
      // No command line flags specified. Look where binary lives
      snprintf(config_file, sizeof(config_file), "%s", CONFIG_FILE);
    } else {
      snprintf(config_file, sizeof(config_file), "%.*s%c%s",
               (int)pos, args[0].c_str(), DIRSEP, CONFIG_FILE);
    }
  }

  fp = fopen(config_file, "r");

  // If config file was set in command line and open failed, die
  if (cmd_line_opts_start == 2 && fp == NULL) {
    die("Cannot open config file %s: %s", config_file, strerror(errno));
  }

  // Load config file settings first
  if (fp != NULL) {
    fprintf(stderr, "Loading config file %s\n", config_file);

    // Loop over the lines in config file
    while (fgets(line, sizeof(line), fp) != NULL) {
      line_no++;

      // Ignore empty lines and comments
      for (i = 0; isspace(* (unsigned char *) &line[i]); ) i++;
      if (line[i] == '#' || line[i] == '\0') {
        continue;
      }

      if (sscanf(line, "%s %[^\r\n#]", opt, val) != 2) {
        printf("%s: line %d is invalid, ignoring it:\n %s",
               config_file, (int) line_no, line);
      } else {
        options[opt] = val;
      }
    }

    (void) fclose(fp);
  }

  // If we're under MacOS and started by launchd, then the second
  // argument is process serial number, -psn_.....
  // In this case, don't process arguments at all.
  if (args.size() >= 2 || memcmp(args[1].c_str(), "-psn_", 5) != 0) {
    // Handle command line flags.
    // They override config file and default settings.
    for (i = cmd_line_opts_start; i < args.size(); i += 2) {
      if (args[i][0] != '-' || i + 1 >= args.size()) {
        show_usage_and_exit();
      }
      options[args[i].substr(1, string::npos)] = args[i + 1];
    }
  }
}

static void init_server_name(void) {
  snprintf(server_name, sizeof(server_name), "Mongoose web server v.%s",
           mg_version());
}

static int event_handler(struct mg_event *event) {
  if (event->type == MG_EVENT_LOG) {
    printf("%s\n", (const char *) event->event_param);
  }
  return 0;
}

static int is_path_absolute(const char *path) {
#ifdef _WIN32
  return path != NULL &&
    ((path[0] == '\\' && path[1] == '\\') ||  // UNC path, e.g. \\server\dir
     (isalpha(path[0]) && path[1] == ':' && path[2] == '\\'));  // E.g. X:\dir
#else
  return path != NULL && path[0] == '/';
#endif
}

static void verify_existence(const map<string, string>& options, const char *option_name,
                             int must_be_dir) {
  struct stat st;

  if (options.count(option_name) != 0 && (stat(options.at(option_name).c_str(), &st) != 0 ||
                       ((S_ISDIR(st.st_mode) ? 1 : 0) != must_be_dir))) {
    die("Invalid path for %s: [%s]: (%s). Make sure that path is either "
        "absolute, or it is relative to mongoose executable.",
        option_name, options.at(option_name).c_str(), strerror(errno));
  }
}

static void set_absolute_path(map<string, string>& options, const char *option_name,
                              const string& path_to_mongoose_exe) {
  char path[PATH_MAX], abs[PATH_MAX];

  // If option is already set and it is an absolute path,
  // leave it as it is -- it's already absolute.
  if (options.count(option_name) != 0 && !is_path_absolute(options[option_name].c_str())) {
    // Not absolute. Use the directory where mongoose executable lives
    // be the relative directory for everything.
    // Extract mongoose executable directory into path.
    size_t pos = path_to_mongoose_exe.rfind(DIRSEP);
    if (pos == string::npos) {
      getcwd(path, sizeof(path));
    } else {
      snprintf(path, sizeof(path), "%.*s", (int)pos,
               path_to_mongoose_exe.c_str());
    }

    strncat(path, "/", sizeof(path) - 1);
    strncat(path, options[option_name].c_str(), sizeof(path) - 1);

    // Absolutize the path, and set the option
    abs_path(path, abs, sizeof(abs));
    options[option_name] = abs;
  }
}

static void start_mongoose(int argc, char *argv[]) {
  vector<string> args;
  for (int i = 0; i < argc; i++) {
    args.push_back(string(argv[i]));
  }
  
  // Edit passwords file if -A option is specified
  if (args.size() >= 2 && args[1].find("-A") == 0) {
    if (args.size() != 6) {
      show_usage_and_exit();
    }
    exit(mg_modify_passwords_file(argv[2], argv[3], argv[4], argv[5]) ?
         EXIT_SUCCESS : EXIT_FAILURE);
  }

  // Show usage if -h or --help options are specified
  if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
    show_usage_and_exit();
  }

  map<string, string> options;
  options["document_root"] = ".";

  // Update config based on command line arguments
  process_command_line_arguments(args, options);

  // Make sure we have absolute paths for files and directories
  // https://github.com/valenok/mongoose/issues/181
  set_absolute_path(options, "document_root", args[0]);
  set_absolute_path(options, "put_delete_auth_file", args[0]);
  set_absolute_path(options, "cgi_interpreter", args[0]);
  set_absolute_path(options, "access_log_file", args[0]);
  set_absolute_path(options, "error_log_file", args[0]);
  set_absolute_path(options, "global_auth_file", args[0]);
  set_absolute_path(options, "ssl_certificate", args[0]);

  // Make extra verification for certain options
  verify_existence(options, "document_root", 1);
  verify_existence(options, "cgi_interpreter", 0);
  verify_existence(options, "ssl_certificate", 0);

  // Setup signal handler: quit on Ctrl-C
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGCHLD, signal_handler);

  // Start Mongoose
  ctx = mg_start(options, event_handler, NULL);

  if (ctx == NULL) {
    die("%s", "Failed to start Mongoose.");
  }
}

int main(int argc, char *argv[]) {
  init_server_name();
  start_mongoose(argc, argv);
  printf("%s started on port(s) %s with web root [%s]\n",
         server_name, mg_get_option(ctx, "listening_ports"),
         mg_get_option(ctx, "document_root"));
  while (exit_flag == 0) {
    sleep(1);
  }
  printf("Exiting on signal %d, waiting for all threads to finish...",
         exit_flag);
  fflush(stdout);
  mg_stop(ctx);
  printf("%s", " done.\n");

  return EXIT_SUCCESS;
}