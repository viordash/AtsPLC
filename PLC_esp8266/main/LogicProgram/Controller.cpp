

#include "LogicProgram/Controller.h"
#include "Datetime/DatetimeService.h"
#include "Display/Common.h"
#include "Display/RenderingService.h"
#include "Display/display.h"
#include "LogicProgram/Ladder.h"
#include "LogicProgram/LogicProgram.h"
#include "WiFi/WiFiService.h"
#include "buttons.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "hotreload_service.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Controller = "controller";

// #define STOP_PROCESS_TASK BIT1
#define STOP_RENDER_TASK BIT2
#define DO_RENDERING BIT3

#define GPIO_EVENTS_ALL_BITS                                                                       \
    (BUTTON_UP_IO_CLOSE | BUTTON_UP_IO_OPEN | BUTTON_DOWN_IO_CLOSE | BUTTON_DOWN_IO_OPEN           \
     | BUTTON_RIGHT_IO_CLOSE | BUTTON_RIGHT_IO_OPEN | BUTTON_SELECT_IO_CLOSE                       \
     | BUTTON_SELECT_IO_OPEN | INPUT_1_IO_CLOSE | INPUT_1_IO_OPEN)

static_assert((Controller::WAKEUP_PROCESS_TASK & GPIO_EVENTS_ALL_BITS) == 0,
              "WAKEUP_PROCESS_TASK must not overlap with any of the sys_gpio event bits");

bool Controller::runned = false;
bool Controller::force_process_loop = false;
EventGroupHandle_t Controller::gpio_events = NULL;
TaskHandle_t Controller::process_task_handle = NULL;

Ladder *Controller::ladder = NULL;
ProcessWakeupService *Controller::processWakeupService = NULL;
WiFiService *Controller::wifi_service = NULL;
RenderingService *Controller::rendering_service = NULL;
DatetimeService *Controller::datetime_service = NULL;

ControllerDI Controller::DI;
ControllerAI Controller::AI;
ControllerDO Controller::O1(gpio_output::OUTPUT_0);
ControllerDO Controller::O2(gpio_output::OUTPUT_1);
ControllerVariable Controller::V1;
ControllerVariable Controller::V2;
ControllerVariable Controller::V3;
ControllerVariable Controller::V4;

void Controller::Start(EventGroupHandle_t gpio_events,
                       WiFiService *wifi_service,
                       RenderingService *rendering_service,
                       DatetimeService *datetime_service) {
    Controller::gpio_events = gpio_events;
    Controller::wifi_service = wifi_service;
    Controller::rendering_service = rendering_service;
    Controller::datetime_service = datetime_service;

    Controller::DI.Init();
    Controller::AI.Init();
    Controller::O1.Init();
    Controller::O2.Init();
    Controller::V1.Init();
    Controller::V2.Init();
    Controller::V3.Init();
    Controller::V4.Init();

    ESP_LOGI(TAG_Controller, "start");

    processWakeupService = new ProcessWakeupService();
    ladder = new Ladder([](int16_t view_top_index, int16_t selected_network) {
        hotreload->view_top_index = view_top_index;
        hotreload->selected_network = selected_network;
        store_hotreload();
        ESP_LOGD(TAG_Controller, "cb_UI_state_changed %d", selected_network);
        Controller::force_process_loop = selected_network > -1;
        if (Controller::force_process_loop) {
            Controller::WakeupProcessTask();
        }
    });

    Controller::runned = true;
    ESP_ERROR_CHECK(xTaskCreate(ProcessTask,
                                "ctrl_actions_task",
                                4096,
                                NULL,
                                3,
                                &Controller::process_task_handle)
                            != pdPASS
                        ? ESP_FAIL
                        : ESP_OK);
}

void Controller::Stop() {
    Controller::runned = false;

    ESP_LOGI(TAG_Controller, "stop");
    const int tasks_stopping_timeout = 500;
    vTaskDelay(tasks_stopping_timeout / portTICK_PERIOD_MS);
    delete ladder;
    delete processWakeupService;
}

void Controller::ProcessTask(void *parm) {
    (void)parm;

    ESP_LOGI(TAG_Controller, "start process task");

    ladder->Load();
    if (hotreload->is_hotstart) {
        ladder->SetViewTopIndex(hotreload->view_top_index);
        ladder->SetSelectedNetworkIndex(hotreload->selected_network);
    }
    ladder->AtLeastOneNetwork();

    rendering_service->Start(ladder);
    xEventGroupClearBits(Controller::gpio_events, GPIO_EVENTS_ALL_BITS | WAKEUP_PROCESS_TASK);

    const uint32_t first_iteration_delay = 0;
    Controller::RequestWakeupMs((void *)Controller::ProcessTask,
                                first_iteration_delay,
                                ProcessWakeupRequestPriority::pwrp_Critical);
    bool repeated_changes = false;
    while (Controller::runned) {
        EventBits_t uxBits = xEventGroupWaitBits(Controller::gpio_events,
                                                 GPIO_EVENTS_ALL_BITS | WAKEUP_PROCESS_TASK,
                                                 true,
                                                 false,
                                                 processWakeupService->Get());

        processWakeupService->RemoveExpired();

        ESP_LOGD(TAG_Controller, "bits:0x%08X", uxBits);
        bool inputs_changed = (uxBits & (INPUT_1_IO_CLOSE | INPUT_1_IO_OPEN));
        bool buttons_changed = !inputs_changed && uxBits != 0;
        bool force_render = false;

        if (buttons_changed) {
            ButtonsPressType pressed_button = handle_buttons(uxBits);
            ESP_LOGD(TAG_Controller, "buttons_changed, pressed_button:%u", pressed_button);
            switch (pressed_button) {
                case ButtonsPressType::UP_PRESSED:
                    ladder->HandleButtonUp();
                    force_render = true;
                    break;
                case ButtonsPressType::UP_LONG_PRESSED:
                    ladder->HandleButtonPageUp();
                    force_render = true;
                    break;
                case ButtonsPressType::DOWN_PRESSED:
                    ladder->HandleButtonDown();
                    force_render = true;
                    break;
                case ButtonsPressType::DOWN_LONG_PRESSED:
                    ladder->HandleButtonPageDown();
                    force_render = true;
                    break;
                case ButtonsPressType::SELECT_PRESSED:
                    ladder->HandleButtonSelect();
                    force_render = true;
                    break;
                case ButtonsPressType::SELECT_LONG_PRESSED:
                    ladder->HandleButtonOption();
                    force_render = true;
                    break;
                default:
                    break;
            }
        }

        FetchIOValues();
        bool any_changes_in_actions = ladder->DoAction();
        CommitChanges();

        if (any_changes_in_actions) {
            ESP_LOGD(TAG_Controller, "any_changes_in_actions %u", repeated_changes);
            Controller::RemoveRequestWakeupMs((void *)Controller::ProcessTask);
            if (!repeated_changes) {
                Controller::RequestWakeupMs((void *)Controller::ProcessTask,
                                            0,
                                            ProcessWakeupRequestPriority::pwrp_Critical);
            } else {
                const uint32_t repeated_changes_cycle_ms = 10;
                Controller::RequestWakeupMs((void *)Controller::ProcessTask,
                                            repeated_changes_cycle_ms,
                                            ProcessWakeupRequestPriority::pwrp_Idle);
            }
            repeated_changes = true;
        } else {
            repeated_changes = false;
            if (Controller::force_process_loop) {
                ESP_LOGD(TAG_Controller, "force_process_loop");
                const uint32_t process_loop_cycle_ms = 200;
                Controller::RequestWakeupMs((void *)Controller::ProcessTask,
                                            process_loop_cycle_ms,
                                            ProcessWakeupRequestPriority::pwrp_Idle);
            }
        }

        if (inputs_changed || any_changes_in_actions || force_render
            || Controller::force_process_loop) {
            rendering_service->Do();
        }
    }

    rendering_service->Stop();
    ESP_LOGI(TAG_Controller, "stop process task");
    vTaskDelete(NULL);
}

void Controller::FetchIOValues() {
    Controller::DI.FetchValue();
    Controller::AI.FetchValue();
    Controller::O1.FetchValue();
    Controller::O2.FetchValue();
    Controller::V1.FetchValue();
    Controller::V2.FetchValue();
    Controller::V3.FetchValue();
    Controller::V4.FetchValue();
}

void Controller::CommitChanges() {
    Controller::O1.CommitChanges();
    Controller::O2.CommitChanges();
    Controller::V1.CommitChanges();
    Controller::V2.CommitChanges();
    Controller::V3.CommitChanges();
    Controller::V4.CommitChanges();
}

bool Controller::RequestWakeupMs(void *id,
                                 uint32_t delay_ms,
                                 ProcessWakeupRequestPriority priority) {
    return processWakeupService->Request(id, delay_ms, priority);
}

void Controller::RemoveRequestWakeupMs(void *id) {
    processWakeupService->RemoveRequest(id);
}

void Controller::RemoveExpiredWakeupRequests() {
    processWakeupService->RemoveExpired();
}

void Controller::BindVariableToSecureWiFi(const MapIO io_adr,
                                          const char *ssid,
                                          const char *password,
                                          const char *mac) {
    if (Controller::wifi_service == NULL) {
        return;
    }
    switch (io_adr) {
        case MapIO::V1:
            Controller::V1.BindToSecureWiFi(Controller::wifi_service, ssid, password, mac);
            break;
        case MapIO::V2:
            Controller::V2.BindToSecureWiFi(Controller::wifi_service, ssid, password, mac);
            break;
        case MapIO::V3:
            Controller::V3.BindToSecureWiFi(Controller::wifi_service, ssid, password, mac);
            break;
        case MapIO::V4:
            Controller::V4.BindToSecureWiFi(Controller::wifi_service, ssid, password, mac);
            break;

        default:
            break;
    }
}

void Controller::BindVariableToInsecureWiFi(const MapIO io_adr, const char *ssid) {
    if (Controller::wifi_service == NULL) {
        return;
    }
    switch (io_adr) {
        case MapIO::V1:
            Controller::V1.BindToInsecureWiFi(Controller::wifi_service, ssid);
            break;
        case MapIO::V2:
            Controller::V2.BindToInsecureWiFi(Controller::wifi_service, ssid);
            break;
        case MapIO::V3:
            Controller::V3.BindToInsecureWiFi(Controller::wifi_service, ssid);
            break;
        case MapIO::V4:
            Controller::V4.BindToInsecureWiFi(Controller::wifi_service, ssid);
            break;

        default:
            break;
    }
}

void Controller::BindVariableToStaWiFi(const MapIO io_adr) {
    if (Controller::wifi_service == NULL) {
        return;
    }
    switch (io_adr) {
        case MapIO::V1:
            Controller::V1.BindToStaWiFi(Controller::wifi_service);
            break;
        case MapIO::V2:
            Controller::V2.BindToStaWiFi(Controller::wifi_service);
            break;
        case MapIO::V3:
            Controller::V3.BindToStaWiFi(Controller::wifi_service);
            break;
        case MapIO::V4:
            Controller::V4.BindToStaWiFi(Controller::wifi_service);
            break;

        default:
            break;
    }
}

void Controller::UnbindVariable(const MapIO io_adr) {
    switch (io_adr) {
        case MapIO::V1:
            Controller::V1.Unbind();
            break;
        case MapIO::V2:
            Controller::V2.Unbind();
            break;
        case MapIO::V3:
            Controller::V3.Unbind();
            break;
        case MapIO::V4:
            Controller::V4.Unbind();
            break;
        default:
            break;
    }
}

void Controller::WakeupProcessTask() {
    xEventGroupSetBits(Controller::gpio_events, WAKEUP_PROCESS_TASK);
}

uint8_t Controller::ConnectToWiFiStation() {
    if (Controller::wifi_service != NULL) {
        return Controller::wifi_service->ConnectToStation();
    }
    return LogicElement::MinValue;
}

void Controller::DisconnectFromWiFiStation() {
    if (Controller::wifi_service != NULL) {
        Controller::wifi_service->DisconnectFromStation();
    }
}