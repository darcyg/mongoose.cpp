//
//  md5context.cpp
//  mongoose
//
//  Created by yayugu on 2013/11/12.
//
//

#include "MD5context.h"
#include "Utils.h"

#include <string.h>

namespace mongoose {
  MD5Context::MD5Context() {
    buf_32[0] = 0x67452301;
    buf_32[1] = 0xefcdab89;
    buf_32[2] = 0x98badcfe;
    buf_32[3] = 0x10325476;
    
    bits[0] = 0;
    bits[1] = 0;
  }
  
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))
  
#define MD5STEP(f, w, x, y, z, data, s) \
( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )
  
  void MD5Context::transform() {
    register uint32_t a, b, c, d;
    //uint32_t *in = (uint32_t *)this->in;
    
    a = buf_32[0];
    b = buf_32[1];
    c = buf_32[2];
    d = buf_32[3];
    
    MD5STEP(F1, a, b, c, d, in_32[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in_32[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in_32[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in_32[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in_32[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in_32[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in_32[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in_32[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in_32[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in_32[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in_32[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in_32[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in_32[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in_32[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in_32[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in_32[15] + 0x49b40821, 22);
    
    MD5STEP(F2, a, b, c, d, in_32[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in_32[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in_32[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in_32[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in_32[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in_32[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in_32[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in_32[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in_32[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in_32[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in_32[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in_32[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in_32[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in_32[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in_32[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in_32[12] + 0x8d2a4c8a, 20);
    
    MD5STEP(F3, a, b, c, d, in_32[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in_32[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in_32[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in_32[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in_32[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in_32[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in_32[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in_32[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in_32[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in_32[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in_32[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in_32[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in_32[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in_32[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in_32[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in_32[2] + 0xc4ac5665, 23);
    
    MD5STEP(F4, a, b, c, d, in_32[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in_32[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in_32[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in_32[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in_32[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in_32[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in_32[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in_32[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in_32[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in_32[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in_32[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in_32[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in_32[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in_32[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in_32[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in_32[9] + 0xeb86d391, 21);
    
    buf_32[0] += a;
    buf_32[1] += b;
    buf_32[2] += c;
    buf_32[3] += d;
  }
  
  void MD5Context::update(unsigned char const *buf, unsigned len) {
    uint32_t t;
    
    t = bits[0];
    if ((bits[0] = t + ((uint32_t) len << 3)) < t)
      bits[1]++;
    bits[1] += len >> 29;
    
    t = (t >> 3) & 0x3f;
    
    if (t) {
      unsigned char *p = (unsigned char *) in_8 + t;
      
      t = 64 - t;
      if (len < t) {
        memcpy(p, buf, len);
        return;
      }
      memcpy(p, buf, t);
      this->byteReverse(16);
      this->transform();
      buf += t;
      len -= t;
    }
    
    while (len >= 64) {
      memcpy(in_8, buf, 64);
      this->byteReverse(16);
      this->transform();
      buf += 64;
      len -= 64;
    }
    
    memcpy(in_32, buf, len);
  }
  
  void MD5Context::byteReverse(unsigned longs) {
    uint32_t t;
    unsigned char *buf_ = buf_8;
    
    // Forrest: MD5 expect LITTLE_ENDIAN, swap if BIG_ENDIAN
    if (is_big_endian()) {
      do {
        t = (uint32_t) ((unsigned) buf_[3] << 8 | buf_[2]) << 16 |
        ((unsigned) buf_[1] << 8 | buf_[0]);
        * (uint32_t *) buf_ = t;
        buf_ += 4;
      } while (--longs);
    }
  }
  
  void MD5Context::final(unsigned char digest[16]) {
    unsigned count;
    unsigned char *p;
    uint32_t *a;
    
    count = (bits[0] >> 3) & 0x3F;
    
    p = in_8 + count;
    *p++ = 0x80;
    count = 64 - 1 - count;
    if (count < 8) {
      memset(p, 0, count);
      this->byteReverse(16);
      this->transform();
      memset(in_32, 0, 56);
    } else {
      memset(p, 0, count - 8);
    }
    this->byteReverse(14);
    
    a = (uint32_t *)in_8;
    a[14] = bits[0];
    a[15] = bits[1];
    
    this->transform();
    this->byteReverse(4);
    memcpy(digest, buf_32, 16);
    memset((char *) this, 0, sizeof(*this));
  }
}