#pragma once

#include <stdint.h>
#include <unistd.h>

enum MapIO : uint8_t { DI = 0, AI, O1, O2, V1, V2, V3, V4 };
extern const char *MapIONames[];

typedef struct { //
    MapIO *inputs_outputs;
    size_t count;
} AllowedIO;

static inline bool ValidateMapIO(MapIO mapIO) {
    if ((int8_t)mapIO < (int8_t)MapIO::DI) {
        return false;
    }
    if ((int8_t)mapIO > (int8_t)MapIO::V4) {
        return false;
    }
    return true;
}

static inline int FindAllowedIO(AllowedIO *allowed, MapIO io) {
    for (size_t i = 0; i < allowed->count; i++) {
        if (allowed->inputs_outputs[i] == io) {
            return i;
        }
    }
    return -1;
}