
#include "http_server.h"
#include "HttpServer.h"
#include "MainController.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

MainController *mainController;
HttpServer *httpServer;

void start_http_server() {
    mainController = new MainController();
    httpServer = new HttpServer({ mainController });
    httpServer->Start();
}

void stop_http_server() {
    httpServer->Stop();
    delete mainController;
    delete httpServer;
}