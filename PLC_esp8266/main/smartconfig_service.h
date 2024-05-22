#pragma once

#include "freertos/event_groups.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

static const int smartconfig_ready_bit = BIT0;

EventGroupHandle_t try_smartconfig();