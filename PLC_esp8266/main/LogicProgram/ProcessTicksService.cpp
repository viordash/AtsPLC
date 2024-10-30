#include "LogicProgram/ProcessTicksService.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ProcessTicksService::ProcessTicksService(/* args */) {
}

ProcessTicksService::~ProcessTicksService() {
}

void ProcessTicksService::Request(uint32_t delay_ms) {
}

uint32_t ProcessTicksService::GetTicksToWait() {
    return 100 / portTICK_PERIOD_MS;
}