#pragma once

#include "freertos/event_groups.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

void start_smartconfig();
bool smartconfig_is_runned();