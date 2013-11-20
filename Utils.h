//
//  Utils.h
//  mongoose
//
//  Created by yayugu on 2013/11/12.
//
//

#ifndef __mongoose__Utils__
#define __mongoose__Utils__

#include <iostream>
#include "internal.h"

namespace mongoose {
  struct mg_context;     // Web server instance
  struct mg_connection;  // HTTP request descriptor
  
  // Unified socket address. For IPv6 support, add IPv6 address structure
  // in the union u.
  union usa {
    struct sockaddr sa;
    struct sockaddr_in sin;
#if defined(USE_IPV6)
    struct sockaddr_in6 sin6;
#endif
  };
  
  // Describes a string (chunk of memory).
  struct vec {
    const char *ptr;
    size_t len;
  };
  
  struct file {
    int is_directory;
    time_t modification_time;
    int64_t size;
    // set to 1 if the content is gzipped
    // in which case we need a content-encoding: gzip header
    int gzipped;
  };
#define STRUCT_FILE_INITIALIZER { 0, 0, 0, 0 }
  
  // Describes listening socket, or socket which was accept()-ed by the master
  // thread and queued for future handling by the worker thread.
  struct socket {
    SOCKET sock;          // Listening socket
    union usa lsa;        // Local socket address
    union usa rsa;        // Remote socket address
    unsigned is_ssl:1;    // Is port SSL-ed
    unsigned ssl_redir:1; // Is port supposed to redirect everything to SSL port
  };
  
  // NOTE(lsm): this enum shoulds be in sync with the config_options.
  enum {
    CGI_EXTENSIONS, CGI_ENVIRONMENT, PUT_DELETE_PASSWORDS_FILE, CGI_INTERPRETER,
    PROTECT_URI, AUTHENTICATION_DOMAIN, SSI_EXTENSIONS, THROTTLE,
    ACCESS_LOG_FILE, ENABLE_DIRECTORY_LISTING, ERROR_LOG_FILE,
    GLOBAL_PASSWORDS_FILE, INDEX_FILES, ENABLE_KEEP_ALIVE, ACCESS_CONTROL_LIST,
    EXTRA_MIME_TYPES, LISTENING_PORTS, DOCUMENT_ROOT, SSL_CERTIFICATE,
    NUM_THREADS, RUN_AS_USER, REWRITE, HIDE_FILES, REQUEST_TIMEOUT,
    NUM_OPTIONS
  };
  
  struct mg_context {
    volatile int stop_flag;         // Should we stop event loop
    SSL_CTX *ssl_ctx;               // SSL context
    char *config[NUM_OPTIONS];      // Mongoose configuration parameters
    mg_event_handler_t event_handler;  // User-defined callback function
    void *user_data;                // User-defined data
    
    struct socket *listening_sockets;
    int num_listening_sockets;
    
    volatile int num_threads;  // Number of threads
    pthread_mutex_t mutex;     // Protects (max|num)_threads
    pthread_cond_t  cond;      // Condvar for tracking workers terminations
    
    struct socket queue[MGSQLEN];   // Accepted sockets
    volatile int sq_head;      // Head of the socket queue
    volatile int sq_tail;      // Tail of the socket queue
    pthread_cond_t sq_full;    // Signaled when socket is produced
    pthread_cond_t sq_empty;   // Signaled when socket is consumed
  };
  
  int is_big_endian(void);
  void mg_strlcpy(register char *dst, register const char *src, size_t n);
  int lowercase(const char *s);
  int mg_strncasecmp(const char *s1, const char *s2, size_t len);
  int mg_strcasecmp(const char *s1, const char *s2);
  char * mg_strndup(const char *ptr, size_t len);
  char * mg_strdup(const char *str);
  const char *mg_strcasestr(const char *big_str, const char *small_str);
  const char *mg_get_builtin_mime_type(std::string const & path);
}

#endif /* defined(__mongoose__Utils__) */
