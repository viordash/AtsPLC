
#include "http_server.h"
#include "DisplayController.h"
#include "HttpServer.h"
#include "InputController.h"
#include "ProgramController.h"
#include "WorkModeController.h"
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
InputController *inputController;
ProgramController *programController;
WorkModeController *workModeController;
HttpServer *httpServer;

void start_http_server() {
    if (http_server_started) {
        stop_http_server();
        return;
    }
    mainController = new MainController();
    updateController = new UpdateController();
    inputController = new InputController();
    programController = new ProgramController();
    workModeController = new WorkModeController();
    std::vector<BaseController *> controllers = { updateController,
                                                  mainController,
                                                  inputController,
                                                  programController,
                                                  workModeController };

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
    delete workModeController;
    delete programController;
    delete inputController;
    delete updateController;
    delete mainController;
    delete httpServer;
    http_server_started = false;
}