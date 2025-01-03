
#include "LogicProgram/Controller.h"
#include "WiFiService.h"
#include "esp_log.h"
#include "esp_smartconfig.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "settings.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>

#define INFINITY_CONNECT_RETRY -1

static const char *TAG_WiFiService_AccessPoint = "WiFiService.AccessPoint";
extern device_settings settings;

bool WiFiService::AccessPointTask(RequestItem *request) {
    ESP_LOGW(TAG_WiFiService_AccessPoint, "start, ssid:%s", request->Payload.Scanner.ssid);

    bool has_new_request = false;
    EventBits_t uxBits = 0;
    bool found = false;
    do {
        uxBits = xEventGroupWaitBits(event,
                                     STOP_BIT | NEW_REQUEST_BIT | CANCEL_REQUEST_BIT,
                                     true,
                                     false,
                                     /*portMAX_DELAY*/ 10000 / portTICK_RATE_MS);
        ESP_LOGI(TAG_WiFiService_AccessPoint, "process, uxBits:0x%08X", uxBits);
        if ((uxBits & NEW_REQUEST_BIT) != 0) {
            has_new_request = true;
        }

        bool timeout = (uxBits & (STOP_BIT | NEW_REQUEST_BIT | CANCEL_REQUEST_BIT)) == 0;
        if (timeout) {
            ESP_LOGI(TAG_WiFiService_AccessPoint, "found");
            found = true;
            break;
        }

    } while (uxBits != 0 && (uxBits & (STOP_BIT | NEW_REQUEST_BIT | CANCEL_REQUEST_BIT)) == 0);

    if (found) {
        Controller::WakeupProcessTask();
        requests.AccessPointDone(request->Payload.Scanner.ssid);
    } else {
        requests.RemoveAccessPoint(request->Payload.Scanner.ssid);
    }

    ESP_LOGW(TAG_WiFiService_AccessPoint, "finish, has_new_request:%u", has_new_request);
    return has_new_request;
}
