#include "service_mode.h"
#include "Maintenance/ServiceModeHandler.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void run_service_mode(EventGroupHandle_t gpio_events) {
    ServiceModeHandler::Start(gpio_events);
}
