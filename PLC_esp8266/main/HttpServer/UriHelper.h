#pragma once

#include <stdint.h>
#include <unistd.h>

class UriHelper {
  public:
    static const char *GetContentType(const char *filename);
};
