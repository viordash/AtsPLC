#pragma once

#include "freertos/event_groups.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

void buttons_init(EventGroupHandle_t gpio_events);
