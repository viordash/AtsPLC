#include "WiFiService.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>

std::unordered_set<WiFiService::RequestItem, WiFiService::RequestItemEqual> WiFiService::requests;

std::mutex WiFiService::lock_mutex;

void WiFiService::Start() {
}

void WiFiService::Stop() {
}

bool WiFiService::Find(const char *ssid) {
    return false;
}

bool WiFiService::Generate(const char *ssid) {
    return false;
}