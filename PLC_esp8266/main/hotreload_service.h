#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef struct {
    bool is_hotstart;
    uint32_t gpio;
    uint32_t restart_count;
} hotreload_data;

extern hotreload_data *hotreload;

void load_hotreload();
void store_hotreload();