
#include "process_engine.h"
#include "LogicProgram/Controller.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void start_process_engine(EventGroupHandle_t gpio_events) {
    Controller::Start(gpio_events);
}

void stop_process_engine() {
    Controller::Stop();
}