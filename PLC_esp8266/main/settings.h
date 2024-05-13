#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef struct {
    size_t size;
    uint32_t state;
    uint32_t version;
} device_settings;

extern device_settings *settings;

void load_settings();
void store_settings();