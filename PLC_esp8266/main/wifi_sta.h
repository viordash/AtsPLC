#pragma once

#include "freertos/event_groups.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

void start_wifi_sta();
void stop_wifi_sta();
bool wifi_sta_is_runned();