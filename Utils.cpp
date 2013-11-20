//
//  Utils.cpp
//  mongoose
//
//  Created by yayugu on 2013/11/12.
//
//

#include <vector>
#include "Utils.h"

namespace mongoose {
  int is_big_endian(void) {
    static const int n = 1;
    return ((char *) &n)[0] == 0;
  }
  
  void mg_strlcpy(register char *dst, register const char *src, size_t n) {
    for (; *src != '\0' && n > 1; n--) {
      *dst++ = *src++;
    }
    *dst = '\0';
  }
  
  int lowercase(const char *s) {
    return tolower(* (const unsigned char *) s);
  }
  
  int mg_strncasecmp(const char *s1, const char *s2, size_t len) {
    int diff = 0;
    
    if (len > 0)
      do {
        diff = lowercase(s1++) - lowercase(s2++);
      } while (diff == 0 && s1[-1] != '\0' && --len > 0);
    
    return diff;
  }
  
  int mg_strcasecmp(const char *s1, const char *s2) {
    int diff;
    
    do {
      diff = lowercase(s1++) - lowercase(s2++);
    } while (diff == 0 && s1[-1] != '\0');
    
    return diff;
  }
  
  char * mg_strndup(const char *ptr, size_t len) {
    char *p;
    
    if ((p = (char *) malloc(len + 1)) != NULL) {
      mg_strlcpy(p, ptr, len + 1);
    }
    
    return p;
  }
  
  char * mg_strdup(const char *str) {
    return mg_strndup(str, strlen(str));
  }
  
  const char *mg_strcasestr(const char *big_str, const char *small_str) {
    int i;
    int big_len = strlen(big_str);
    int small_len = strlen(small_str);
    
    for (i = 0; i <= big_len - small_len; i++) {
      if (mg_strncasecmp(big_str + i, small_str, small_len) == 0) {
        return big_str + i;
      }
    }
    
    return NULL;
  }
  
  struct BuilltInMimeType {
    const char *extension;
    size_t ext_len;
    const char *mime_type;
  };
  
  static const struct BuilltInMimeType builtin_mime_types[] = {
    {".html", 5, "text/html"},
    {".htm", 4, "text/html"},
    {".shtm", 5, "text/html"},
    {".shtml", 6, "text/html"},
    {".css", 4, "text/css"},
    {".js",  3, "application/x-javascript"},
    {".ico", 4, "image/x-icon"},
    {".gif", 4, "image/gif"},
    {".jpg", 4, "image/jpeg"},
    {".jpeg", 5, "image/jpeg"},
    {".png", 4, "image/png"},
    {".svg", 4, "image/svg+xml"},
    {".txt", 4, "text/plain"},
    {".torrent", 8, "application/x-bittorrent"},
    {".wav", 4, "audio/x-wav"},
    {".mp3", 4, "audio/x-mp3"},
    {".mid", 4, "audio/mid"},
    {".m3u", 4, "audio/x-mpegurl"},
    {".ogg", 4, "application/ogg"},
    {".ram", 4, "audio/x-pn-realaudio"},
    {".xml", 4, "text/xml"},
    {".json",  5, "text/json"},
    {".xslt", 5, "application/xml"},
    {".xsl", 4, "application/xml"},
    {".ra",  3, "audio/x-pn-realaudio"},
    {".doc", 4, "application/msword"},
    {".exe", 4, "application/octet-stream"},
    {".zip", 4, "application/x-zip-compressed"},
    {".xls", 4, "application/excel"},
    {".tgz", 4, "application/x-tar-gz"},
    {".tar", 4, "application/x-tar"},
    {".gz",  3, "application/x-gunzip"},
    {".arj", 4, "application/x-arj-compressed"},
    {".rar", 4, "application/x-arj-compressed"},
    {".rtf", 4, "application/rtf"},
    {".pdf", 4, "application/pdf"},
    {".swf", 4, "application/x-shockwave-flash"},
    {".mpg", 4, "video/mpeg"},
    {".webm", 5, "video/webm"},
    {".mpeg", 5, "video/mpeg"},
    {".mov", 4, "video/quicktime"},
    {".mp4", 4, "video/mp4"},
    {".m4v", 4, "video/x-m4v"},
    {".asf", 4, "video/x-ms-asf"},
    {".avi", 4, "video/x-msvideo"},
    {".bmp", 4, "image/bmp"},
    {".ttf", 4, "application/x-font-ttf"}
  };
  
  const char *mg_get_builtin_mime_type(std::string const & path) {
    static const std::vector<BuilltInMimeType> mime_types(builtin_mime_types,
                                                          builtin_mime_types + sizeof(builtin_mime_types) / sizeof(BuilltInMimeType));
    const char *ext;
    
    for (const BuilltInMimeType & mime_type : mime_types) {
      ext = path.c_str() + (path.length() - mime_type.ext_len);
      if (path.length() > mime_type.ext_len &&
          mg_strcasecmp(ext, mime_type.extension) == 0) {
        return mime_type.mime_type;
      }
    }
    
    return "text/plain";
  }
}