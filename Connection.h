//
//  Connection.h
//  mongoose
//
//  Created by yayugu on 2013/11/13.
//
//

#ifndef __mongoose__Connection__
#define __mongoose__Connection__

#include "internal.h"
#include "Utils.h"

namespace mongoose {
  struct Connection {
    struct mg_request_info request_info;
    struct mg_event event;
    struct mg_context *ctx;
    SSL *ssl;                   // SSL descriptor
    SSL_CTX *client_ssl_ctx;    // SSL context for client connections
    struct socket client;       // Connected client
    time_t birth_time;          // Time when request was received
    int64_t num_bytes_sent;     // Total bytes sent to client
    int64_t content_len;        // Content-Length header value
    int64_t num_bytes_read;     // Bytes read from a remote socket
    char *buf;                  // Buffer for received data
    char *path_info;            // PATH_INFO part of the URL
    int must_close;             // 1 if connection must be closed
    int buf_size;               // Buffer size
    int request_len;            // Size of the request + headers in a buffer
    int data_len;               // Total size of data in a buffer
    int status_code;            // HTTP reply status code, e.g. 200
    int throttle;               // Throttling, bytes/sec. <= 0 means no throttle
    time_t last_throttle_time;  // Last time throttled data was sent
    int64_t last_throttle_bytes;// Bytes sent this second
  };
}

#endif /* defined(__mongoose__Connection__) */
