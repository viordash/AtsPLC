#include "MainController.h"
#include "UriHelper.h"
#include "esp_log.h"
#include "lassert.h"
#include <algorithm>
#include <sys/param.h>

static const char *TAG_MainController = "main_controller";

struct WebFile {
    const uint8_t *data;
    const size_t data_size;
    const char *name;
};

#define EMBEDDED_FILE_SYMBOL(symbol, base_name) EMBEDDED_FILE_SYMBOL_IMPL(symbol, base_name)
#define EMBEDDED_FILE_SYMBOL_IMPL(symbol, base_name)                                               \
    extern uint8_t _binary_##symbol##_start[];                                                     \
    extern uint8_t _binary_##symbol##_end;                                                         \
    static const WebFile base_name##_file = { .data = _binary_##symbol##_start,                    \
                                              .data_size = (size_t)(&_binary_##symbol##_end        \
                                                                    - _binary_##symbol##_start),   \
                                              .name = base_name##_name };

#define URI_HANDLER(file)                                                                          \
    { .uri = "/" file##_name,                                                                      \
      .method = HTTP_GET,                                                                          \
      .handler = MainController::SendingFile,                                                      \
      .user_ctx = (void *)&file##_file }

EMBEDDED_FILE_SYMBOL(index_html_symbol, index_html)
EMBEDDED_FILE_SYMBOL(favicon_ico_symbol, favicon_ico)
EMBEDDED_FILE_SYMBOL(main_js_symbol, main_js)
EMBEDDED_FILE_SYMBOL(styles_css_symbol, styles_css)

MainController::MainController() : BaseController() {
}

MainController::~MainController() {
}

size_t MainController::GetUriHandlers(httpd_uri_t *handlers, size_t capacity) {
    const size_t count = 6;
    ASSERT(capacity >= count);
    handlers[0] = { .uri = "/",
                    .method = HTTP_GET,
                    .handler = MainController::SendingFile,
                    .user_ctx = (void *)&index_html_file };
    handlers[1] = { .uri = "/main",
                    .method = HTTP_GET,
                    .handler = MainController::SendingFile,
                    .user_ctx = (void *)&index_html_file };
    handlers[2] = URI_HANDLER(index_html);
    handlers[3] = URI_HANDLER(favicon_ico);
    handlers[4] = URI_HANDLER(main_js);
    handlers[5] = URI_HANDLER(styles_css);
    return count;
}

esp_err_t MainController::SendingFile(httpd_req_t *req) {
    auto web_file = static_cast<WebFile *>(req->user_ctx);

    const uint8_t *data = web_file->data;
    size_t data_size = web_file->data_size;
    const char *filename = web_file->name;
    size_t pos = 0;

    ESP_LOGI(TAG_MainController,
             "Sending file : %s (%u bytes)...",
             filename,
             (unsigned int)data_size);
    esp_err_t res = httpd_resp_set_type(req, UriHelper::GetContentType(filename));
    if (res != ESP_OK) {
        return res;
    }

    res = httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=3600");
    if (res != ESP_OK) {
        return res;
    }

    do {
        size_t chunk_size = std::min(data_size, (size_t)SCRATCH_BUFSIZE);
        if (httpd_resp_send_chunk(req, (const char *)&data[pos], chunk_size) != ESP_OK) {
            ESP_LOGE(TAG_MainController, "File sending failed!");
            httpd_resp_send_chunk(req, NULL, 0);
            SendError(req, "500 Server Error", "File sending failed");
            break;
        }
        pos += chunk_size;
        data_size -= chunk_size;
    } while (data_size > 0);

    if (data_size != 0) {
        return ESP_FAIL;
    }
    return httpd_resp_send_chunk(req, NULL, 0);
}
