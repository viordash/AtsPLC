#pragma once

#include <stdint.h>
#include <unistd.h>

typedef struct {
    int32_t is_hotstart;
    uint32_t restart_count;
    int32_t view_top_index;
    int32_t selected_network;
} hotreload_data;

extern hotreload_data *hotreload;

void load_hotreload();
void store_hotreload();