#include "WorkModeController.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Ladder.h"
#include "esp_log.h"
#include "hotreload_service.h"
#include "lassert.h"
#include "sys_gpio.h"
#include <stdio.h>

static const char *TAG_WorkModeController = "WorkModeController";

WorkModeController::WorkModeController() : BaseController() {
}

WorkModeController::~WorkModeController() {
}

size_t WorkModeController::GetUriHandlers(httpd_uri_t *handlers, size_t capacity) {
    const size_t count = 2;
    ASSERT(capacity >= count);
    handlers[0] = { .uri = "/workmode",
                    .method = HTTP_GET,
                    .handler = WorkModeController::GetWorkMode,
                    .user_ctx = this };
    handlers[1] = { .uri = "/workmode",
                    .method = HTTP_POST,
                    .handler = WorkModeController::SetWorkMode,
                    .user_ctx = this };
    return count;
}

EffectiveWorkMode WorkModeController::GetCurrentMode() {
    bool enable_debug;
    SAFETY_HOTRELOAD({ enable_debug = hotreload->enable_debug; });

    return Convert2EffectiveWorkMode(Controller::GetLadder().GetWorkMode(), enable_debug);
}

EventBits_t WorkModeController::Convert2Events(EffectiveWorkMode mode) {
    switch (mode) {
        case EffectiveWorkMode::ewm_Stop:
            return WORK_MODES_0;

        case EffectiveWorkMode::ewm_Run:
            return WORK_MODES_1;

        case EffectiveWorkMode::ewm_Debug:
            return WORK_MODES_0 | WORK_MODES_1;
    }
    return WORK_MODES_0;
}

esp_err_t WorkModeController::GetWorkMode(httpd_req_t *req) {
    char response[32];
    int size =
        snprintf(response, sizeof(response), "{\"mode\":%u}", (unsigned int)GetCurrentMode());

    ESP_LOGD(TAG_WorkModeController, "GetWorkMode : %s", response);
    esp_err_t res = httpd_resp_set_type(req, HTTPD_TYPE_JSON);
    if (res != ESP_OK) {
        return res;
    }
    return httpd_resp_send(req, response, size);
}

esp_err_t WorkModeController::SetWorkMode(httpd_req_t *req) {
    char buffer[64];

    int ret = httpd_req_recv(req, buffer, sizeof(buffer) - 1);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            SendError(req, "408 Request Timeout", "Server closed this connection");
        }
        return ESP_FAIL;
    }
    buffer[ret] = '\0';

    int requested_mode;
    int parsed = sscanf(buffer, "{\"mode\":%d}", &requested_mode);
    EffectiveWorkMode mode = (EffectiveWorkMode)requested_mode;
    if (parsed != 1 || !ValidateEffectiveWorkMode(mode)) {
        ESP_LOGE(TAG_WorkModeController, "SetWorkMode, unexpected body: %s", buffer);
        SendError(req, "400 Bad Request", "unexpected body");
        return ESP_OK;
    }

    if (Controller::InDesign()) {
        SendError(req, "400 Bad Request", "device is being edited, work mode is locked");
        return ESP_OK;
    }

    ESP_LOGI(TAG_WorkModeController, "SetWorkMode : %u", (unsigned int)mode);
    xEventGroupSetBits(Controller::gpio_events, Convert2Events(mode));

    esp_err_t res = httpd_resp_send(req, NULL, 0);
    if (res != ESP_OK) {
        ESP_LOGE(TAG_WorkModeController, "SetWorkMode httpd_resp_send error");
    }
    return res;
}
