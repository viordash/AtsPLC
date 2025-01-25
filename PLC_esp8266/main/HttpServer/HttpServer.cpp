
#include "HttpServer.h"
#include <esp_event.h>
#include <esp_log.h>
#include <signal.h>
#include <sys/param.h>
#include <unistd.h>

static const char *TAG_HttpServer = "http_server";

HttpServer::HttpServer(std::vector<BaseController *> controllers) {
    this->controllers = controllers;
}

HttpServer::~HttpServer() {
    Stop();
}

bool HttpServer::Start() {
    httpd_config_t config = { //
                              .task_priority = tskIDLE_PRIORITY + 5,
                              .stack_size = 4096,
                              .server_port = 80,
                              .ctrl_port = 32768,
                              .max_open_sockets = 7,
                              .max_uri_handlers = 8,
                              .max_resp_headers = 8,
                              .backlog_conn = 5,
                              .lru_purge_enable = false,
                              .recv_wait_timeout = 5,
                              .send_wait_timeout = 5,
                              .global_user_ctx = NULL,
                              .global_user_ctx_free_fn = NULL,
                              .global_transport_ctx = NULL,
                              .global_transport_ctx_free_fn = NULL,
                              .open_fn = NULL,
                              .close_fn = NULL
    };

    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG_HttpServer, "Start error");
        server = NULL;
        return false;
    }
    ESP_LOGI(TAG_HttpServer, "Starting, listen port:%u", config.server_port);

    for (const auto &controller : controllers) {
        for (const auto &uriHandler : controller->GetUriHandlers()) {
            ESP_LOGI(TAG_HttpServer, "reg URI:%s", uriHandler->uri);
            httpd_register_uri_handler(server, uriHandler);
        }
    }
    return true;
}

void HttpServer::Stop() {
    if (server != NULL) {
        httpd_stop(server);
        ESP_LOGI(TAG_HttpServer, "Stopped");
    }
    server = NULL;
}
