
#include "http_server.h"
#include "HttpServer.h"
#include "MainController.h"
#include "UpdateController.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

MainController *mainController;
UpdateController *updateController;
HttpServer *httpServer;

void start_http_server() {
    mainController = new MainController();
    updateController = new UpdateController();
    httpServer = new HttpServer({ updateController, mainController });
    httpServer->Start();
}

void stop_http_server() {
    httpServer->Stop();
    delete updateController;
    delete mainController;
    delete httpServer;
}