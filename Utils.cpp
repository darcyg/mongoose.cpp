//
//  Utils.cpp
//  mongoose
//
//  Created by yayugu on 2013/11/12.
//
//

#include "Utils.h"

namespace mongoose {
  int is_big_endian(void) {
    static const int n = 1;
    return ((char *) &n)[0] == 0;
  }
}