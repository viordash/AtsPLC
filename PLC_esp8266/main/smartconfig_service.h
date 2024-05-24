#pragma once

#include "freertos/event_groups.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>


void try_smartconfig();
bool smartconfig_is_runned();
bool smartconfig_has_ready(TickType_t xTicksToWait);
void stop_smartconfig();