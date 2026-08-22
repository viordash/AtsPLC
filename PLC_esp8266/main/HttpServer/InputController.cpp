#include "InputController.h"
#include "LogicProgram/Controller.h"
#include "esp_log.h"
#include "lassert.h"
#include "sys_gpio.h"
#include <stdio.h>

static const char *TAG_InputController = "InputController";

InputController::InputController() : BaseController() {
}

InputController::~InputController() {
}

size_t InputController::GetUriHandlers(httpd_uri_t *handlers, size_t capacity) {
    const size_t count = 1;
    ASSERT(capacity >= count);
    handlers[0] = { .uri = "/keypress",
                    .method = HTTP_POST,
                    .handler = InputController::KeyPress,
                    .user_ctx = this };
    return count;
}

esp_err_t InputController::KeyPress(httpd_req_t *req) {
    char buffer[256];

    int ret = httpd_req_recv(req, buffer, sizeof(buffer) - 1);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            SendError(req, "408 Request Timeout", "Server closed this connection");
        }
        return ESP_FAIL;
    }
    buffer[ret] = '\0';

    int key_code;
    int keydown;
    int parsed = sscanf(buffer, "{\"key\":%d,\"down\":%d}", &key_code, &keydown);
    if (parsed != 2) {
        SendError(req, "400 Bad Request", "unexpected body");
        return ESP_OK;
    }

    ESP_LOGD(TAG_InputController, "KeyPress key: %d, down:%d", key_code, keydown);

    const int ArrowUp = 38;
    const int ArrowDown = 40;
    const int ShiftRight = 16;

    switch (key_code) {
        case ArrowUp:
            xEventGroupSetBits(Controller::gpio_events,
                               keydown ? BUTTON_UP_IO_CLOSE : BUTTON_UP_IO_OPEN);
            break;
        case ArrowDown:
            xEventGroupSetBits(Controller::gpio_events,
                               keydown ? BUTTON_DOWN_IO_CLOSE : BUTTON_DOWN_IO_OPEN);
            break;
        case ShiftRight:
            xEventGroupSetBits(Controller::gpio_events,
                               keydown ? BUTTON_SELECT_IO_CLOSE : BUTTON_SELECT_IO_OPEN);
            break;

        default:
            break;
    }

    esp_err_t res = httpd_resp_send(req, NULL, 0);
    if (res != ESP_OK) {
        ESP_LOGE(TAG_InputController, "KeyPress httpd_resp_send error");
        return res;
    }

    return ESP_OK;
}
