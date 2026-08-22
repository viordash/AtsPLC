
#include "http_server.h"
#include "DisplayController.h"
#include "HttpServer.h"
#include "LogicProgram/Controller.h"
#include "MainController.h"
#include "UpdateController.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static bool http_server_started = false;
MainController *mainController;
UpdateController *updateController;
DisplayController *displayController;
HttpServer *httpServer;

void start_http_server() {
    if (http_server_started) {
        stop_http_server();
        return;
    }
    RenderingService *rendering_service = Controller::GetRenderingService();
    if (rendering_service == NULL) {
        return;
    }

    mainController = new MainController();
    updateController = new UpdateController();
    displayController = new DisplayController(*rendering_service);
    httpServer = new HttpServer({ updateController, mainController, displayController });
    httpServer->Start();
    http_server_started = true;
}

void stop_http_server() {
    if (!http_server_started) {
        return;
    }
    httpServer->Stop();
    delete displayController;
    delete updateController;
    delete mainController;
    delete httpServer;
    http_server_started = false;
}