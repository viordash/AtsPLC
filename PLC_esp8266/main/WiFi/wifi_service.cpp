
#include "wifi_service.h"
#include "WiFiService.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

WiFiService *wifi_service;

void start_wifi_service() {
    wifi_service = new WiFiService();
}

void stop_wifi_service() {
    delete wifi_service;
}