//
//  SHA1Context.h
//  mongoose
//
//  Created by yayugu on 2013/11/21.
//
//

#ifndef __mongoose__SHA1Context__
#define __mongoose__SHA1Context__

#include <iostream>

namespace mongoose {
  struct SHA1Context {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
    
    SHA1Context();
    void update(const unsigned char* data, uint32_t len);
    void final(unsigned char digest[20]);
  };
}

#endif /* defined(__mongoose__SHA1Context__) */
