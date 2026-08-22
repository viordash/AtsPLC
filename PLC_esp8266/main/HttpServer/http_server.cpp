
#include "http_server.h"
#include "DisplayController.h"
#include "HttpServer.h"
#include "LogicProgram/Controller.h"
#include "MainController.h"
#include "UpdateController.h"
#include <stdint.h>
#include <vector>
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
    mainController = new MainController();
    updateController = new UpdateController();
    std::vector<BaseController *> controllers = { updateController, mainController };

    RenderingService *rendering_service = Controller::GetRenderingService();
    displayController = NULL;
    if (rendering_service != NULL) {
        displayController = new DisplayController(*rendering_service);
        controllers.push_back(displayController);
    }

    httpServer = new HttpServer(controllers);
    httpServer->Start();
    http_server_started = true;
}

void stop_http_server() {
    if (!http_server_started) {
        return;
    }
    httpServer->Stop();
    if (displayController != NULL) {
        delete displayController;
        displayController = NULL;
    }
    delete updateController;
    delete mainController;
    delete httpServer;
    http_server_started = false;
}