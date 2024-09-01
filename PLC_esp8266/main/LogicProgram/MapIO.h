#pragma once

#include <stdint.h>
#include <unistd.h>

enum MapIO : uint8_t { DI = 0, AI, O1, O2, V1, V2, V3, V4 };
extern const char *MapIONames[];
