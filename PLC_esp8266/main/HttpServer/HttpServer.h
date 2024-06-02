#pragma once

#include "BaseController.h"
#include <esp_http_server.h>
#include <vector>

class HttpServer {
  public:
    explicit HttpServer(std::vector<BaseController *> controllers);
    ~HttpServer();

    bool Start();
    void Stop();

  private:
    std::vector<BaseController *> controllers;
    httpd_handle_t server = NULL;
};
