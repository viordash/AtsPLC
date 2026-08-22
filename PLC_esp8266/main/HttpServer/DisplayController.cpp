#include "DisplayController.h"
#include "Display/display.h"
#include "LogicProgram/Controller.h"
#include "config.h"
#include "esp_log.h"
#include "lassert.h"
#include <stdio.h>
#include <string.h>

static const char *TAG_DisplayController = "DisplayController";

DisplayController::DisplayController(RenderingService &rendering_service)
    : BaseController(), rendering_service{ rendering_service } {
}

DisplayController::~DisplayController() {
}

size_t DisplayController::GetUriHandlers(httpd_uri_t *handlers, size_t capacity) {
    const size_t count = 2;
    ASSERT(capacity >= count);
    handlers[0] = { .uri = "/devconfig",
                    .method = HTTP_GET,
                    .handler = DisplayController::GetDeviceConfig,
                    .user_ctx = this };
    handlers[1] = { .uri = "/bitmap",
                    .method = HTTP_GET,
                    .handler = DisplayController::GetBitmap,
                    .user_ctx = this };
    return count;
}

esp_err_t DisplayController::GetDeviceConfig(httpd_req_t *req) {
    char response[256];
    int size = snprintf(response,
                        sizeof(response),
                        "{\"display\":{\"height\":%u,\"width\":%u,\"id\":\"%s\",\"area_top\":%u,"
                        "\"area_width\":%u,\"rails_height\":%u},\"request_limit\":%u}",
                        DISPLAY_HEIGHT,
                        DISPLAY_WIDTH,
                        DISPLAY_ID,
                        INCOME_RAIL_TOP,
                        SCROLLBAR_LEFT,
                        INCOME_RAIL_HEIGHT,
                        Http_RequestsLimit);

    ESP_LOGD(TAG_DisplayController, "GetDeviceConfig : %s", response);
    esp_err_t res = httpd_resp_set_type(req, HTTPD_TYPE_JSON);
    if (res != ESP_OK) {
        return res;
    }
    return httpd_resp_send(req, response, size);
}

esp_err_t DisplayController::GetBitmap(httpd_req_t *req) {
    auto controller = static_cast<DisplayController *>(req->user_ctx);
    ESP_LOGD(TAG_DisplayController, "GetBitmap request received");

    esp_err_t res;
    char etag_s[64];
    char if_none_match[64];

    res = httpd_req_get_hdr_value_str(req, "If-None-Match", if_none_match, sizeof(if_none_match));
    if (res != ESP_OK) {
        if_none_match[0] = 0;
    }

    auto cached_bitmap = controller->rendering_service.BeginRenderOnExternal();
    snprintf(etag_s, sizeof(etag_s), "%u", (unsigned int)cached_bitmap.last_change_time_ms);

    if (strcmp(if_none_match, etag_s) != 0) {
        res = controller->CreateBitmapDataResponse(req,
                                                   cached_bitmap.bitmap,
                                                   etag_s,
                                                   cached_bitmap.view_offset,
                                                   cached_bitmap.view_count);
    } else {
        res = controller->CreateNotModifiedResponse(req, etag_s);
    }
    controller->rendering_service.EndRenderOnExternal();
    return res;
}

esp_err_t DisplayController::CreateBitmapDataResponse(httpd_req_t *req,
                                                      uint8_t *bitmap,
                                                      const char *etag,
                                                      int32_t view_offset,
                                                      uint32_t view_count) {
    ESP_LOGD(TAG_DisplayController, "CreateBitmapDataResponse");
    esp_err_t res = httpd_resp_set_type(req, HTTPD_TYPE_OCTET);
    if (res != ESP_OK) {
        ESP_LOGE(TAG_DisplayController, "CreateBitmapDataResponse httpd_resp_set_type error");
        return res;
    }

    res = httpd_resp_set_hdr(req, "ETag", etag);
    if (res != ESP_OK) {
        ESP_LOGE(TAG_DisplayController, "CreateBitmapDataResponse httpd_resp_set_hdr 'ETag' error");
        return res;
    }

    char data_paging[256];
    snprintf(data_paging,
             sizeof(data_paging),
             "{\"offset\":%d,\"count\":%u}",
             (signed)view_offset,
             (unsigned)view_count);
    res = httpd_resp_set_hdr(req, "X-DataPaging", data_paging);
    if (res != ESP_OK) {
        ESP_LOGE(TAG_DisplayController,
                 "CreateBitmapDataResponse httpd_resp_set_hdr 'X-DataPaging' error");
        return res;
    }

    char force_refresh[16];
    res = SetForceRefreshHeader(req, force_refresh, sizeof(force_refresh));
    if (res != ESP_OK) {
        ESP_LOGE(TAG_DisplayController,
                 "CreateBitmapDataResponse httpd_resp_set_hdr 'X-ForceRefresh' error");
        return res;
    }

    res = httpd_resp_send(req, (const char *)bitmap, DISPLAY_HEIGHT_IN_BYTES * DISPLAY_WIDTH);
    if (res != ESP_OK) {
        ESP_LOGE(TAG_DisplayController, "CreateBitmapDataResponse httpd_resp_send error");
        return res;
    }
    return res;
}

esp_err_t DisplayController::CreateNotModifiedResponse(httpd_req_t *req, const char *etag) {
    ESP_LOGD(TAG_DisplayController, "CreateNotModifiedResponse");
    esp_err_t res = httpd_resp_set_status(req, "304 Not Modified");
    if (res != ESP_OK) {
        ESP_LOGE(TAG_DisplayController, "CreateNotModifiedResponse httpd_resp_set_status error");
        return res;
    }

    res = httpd_resp_set_hdr(req, "ETag", etag);
    if (res != ESP_OK) {
        ESP_LOGE(TAG_DisplayController, "CreateNotModifiedResponse httpd_resp_set_hdr 'ETag' error");
        return res;
    }

    char force_refresh[16];
    res = SetForceRefreshHeader(req, force_refresh, sizeof(force_refresh));
    if (res != ESP_OK) {
        ESP_LOGE(TAG_DisplayController,
                 "CreateNotModifiedResponse httpd_resp_set_hdr 'X-ForceRefresh' error");
        return res;
    }

    res = httpd_resp_send(req, NULL, 0);
    if (res != ESP_OK) {
        ESP_LOGE(TAG_DisplayController, "CreateNotModifiedResponse httpd_resp_send error");
        return res;
    }
    return res;
}

esp_err_t DisplayController::SetForceRefreshHeader(httpd_req_t *req, char *dst, size_t size) {
    ForceRefreshUI force_refresh = Controller::TakeForceRefreshUI();

    switch (force_refresh) {
        case ForceRefreshUI::fru_None:
            return ESP_OK;

        case ForceRefreshUI::fru_WorkMode:
            snprintf(dst, size, "%u", (unsigned int)force_refresh);
            return httpd_resp_set_hdr(req, "X-ForceRefresh", dst);
    }
    return ESP_FAIL;
}
