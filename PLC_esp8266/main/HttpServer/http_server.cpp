
#include "http_server.h"
#include "HttpServer.h"
#include "MainController.h"
#include "UpdateController.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static bool http_server_started = false;
MainController *mainController;
UpdateController *updateController;
HttpServer *httpServer;

void start_http_server() {
    if (http_server_started) {
        stop_http_server();
        return;
    }
    mainController = new MainController();
    updateController = new UpdateController();
    httpServer = new HttpServer({ updateController, mainController });
    httpServer->Start();
    http_server_started = true;
}

void stop_http_server() {
    if (!http_server_started) {
        return;
    }
    httpServer->Stop();
    delete updateController;
    delete mainController;
    delete httpServer;
    http_server_started = false;
}