#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef struct {
    bool is_hotstart;
    uint32_t restart_count;
    int16_t view_top_index;
} hotreload_data;

extern hotreload_data *hotreload;

void load_hotreload();
void store_hotreload();