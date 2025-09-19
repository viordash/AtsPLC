

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
     | BUTTON_SELECT_IO_CLOSE | BUTTON_SELECT_IO_OPEN | INPUT_1_IO_CLOSE | INPUT_1_IO_OPEN)

static_assert((Controller::WAKEUP_PROCESS_TASK & GPIO_EVENTS_ALL_BITS) == 0,
              "WAKEUP_PROCESS_TASK must not overlap with any of the sys_gpio event bits");

extern CurrentSettings::device_settings settings;

bool Controller::runned = false;
bool Controller::in_design = false;
EventGroupHandle_t Controller::gpio_events = NULL;
TaskHandle_t Controller::process_task_handle = NULL;

ProcessWakeupService *Controller::processWakeupService = NULL;
WiFiService *Controller::wifi_service = NULL;
RenderingService *Controller::rendering_service = NULL;
DatetimeService *Controller::datetime_service = NULL;
LogicItemState Controller::network_continuation = LogicItemState::lisPassive;

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
    delete processWakeupService;
}

void Controller::ProcessTask(void *parm) {
    (void)parm;

    ESP_LOGI(TAG_Controller, "start process task");

    network_continuation = LogicItemState::lisPassive;

    Ladder ladder;
    ladder.Load();
    if (hotreload->is_hotstart) {
        ladder.SetViewTopIndex(hotreload->view_top_index);
        ladder.SetSelectedNetworkIndex(hotreload->selected_network);
    }
    ladder.AtLeastOneNetwork();

    rendering_service->Start(&ladder);
    xEventGroupClearBits(Controller::gpio_events, GPIO_EVENTS_ALL_BITS | WAKEUP_PROCESS_TASK);

    const uint32_t first_iteration_delay = 0;
    Controller::RequestWakeupMs((void *)Controller::ProcessTask,
                                first_iteration_delay,
                                ProcessWakeupRequestPriority::pwrp_Critical);
    while (Controller::runned) {
        EventBits_t uxBits = xEventGroupWaitBits(Controller::gpio_events,
                                                 GPIO_EVENTS_ALL_BITS | WAKEUP_PROCESS_TASK,
                                                 true,
                                                 false,
                                                 Controller::GetWakeupTicks());

        Controller::RemoveExpiredWakeupRequests();

        ESP_LOGD(TAG_Controller, "bits:0x%08X", (unsigned int)uxBits);
        bool inputs_changed = (uxBits & (INPUT_1_IO_CLOSE | INPUT_1_IO_OPEN));
        bool buttons_changed = !inputs_changed && uxBits != 0;
        bool do_render = inputs_changed;

        if (buttons_changed) {
            ButtonsPressType pressed_button = handle_buttons(uxBits);
            ESP_LOGD(TAG_Controller, "buttons_changed, pressed_button:%u", pressed_button);
            switch (pressed_button) {
                case ButtonsPressType::UP_PRESSED:
                    ladder.HandleButtonUp();
                    do_render = true;
                    break;
                case ButtonsPressType::UP_LONG_PRESSED:
                    ladder.HandleButtonPageUp();
                    do_render = true;
                    break;
                case ButtonsPressType::DOWN_PRESSED:
                    ladder.HandleButtonDown();
                    do_render = true;
                    break;
                case ButtonsPressType::DOWN_LONG_PRESSED:
                    ladder.HandleButtonPageDown();
                    do_render = true;
                    break;
                case ButtonsPressType::SELECT_PRESSED:
                    ladder.HandleButtonSelect();
                    do_render = true;
                    break;
                case ButtonsPressType::SELECT_LONG_PRESSED:
                    ladder.HandleButtonOption();
                    do_render = true;
                    break;
                default:
                    break;
            }
        }

        bool looped_actions = false;
        auto cycle_start_time = (uint64_t)esp_timer_get_time();
        const uint64_t max_cycle_ms = portTICK_PERIOD_MS * 2;
        auto next_time = cycle_start_time + (max_cycle_ms * 1000);
        bool expired;
        do {
            FetchIOValues();
            bool any_changes_in_actions = ladder.DoAction();
            CommitChanges();
            if (!any_changes_in_actions) {
                break;
            }
            int64_t timespan = next_time - (uint64_t)esp_timer_get_time();
            expired = timespan <= (portTICK_PERIOD_MS / 2) * 1000;
            do_render = true;
            looped_actions = expired;
        } while (!expired);

        if (looped_actions) {
            ESP_LOGD(TAG_Controller, "looped actions");
            Controller::RemoveRequestWakeupMs((void *)Controller::ProcessTask);
            const uint32_t loop_break_ms = portTICK_PERIOD_MS * 2;
            Controller::RequestWakeupMs((void *)Controller::ProcessTask,
                                        loop_break_ms,
                                        ProcessWakeupRequestPriority::pwrp_Critical);
        }

        if (do_render || Controller::in_design) {
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

uint32_t Controller::GetWakeupTicks() {
    return processWakeupService->Get();
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

void Controller::BindVariableToToDateTime(const MapIO io_adr, DatetimePart datetime_part) {
    if (Controller::datetime_service == NULL) {
        return;
    }
    switch (io_adr) {
        case MapIO::V1:
            Controller::V1.BindToDateTime(Controller::datetime_service, datetime_part);
            break;
        case MapIO::V2:
            Controller::V2.BindToDateTime(Controller::datetime_service, datetime_part);
            break;
        case MapIO::V3:
            Controller::V3.BindToDateTime(Controller::datetime_service, datetime_part);
            break;
        case MapIO::V4:
            Controller::V4.BindToDateTime(Controller::datetime_service, datetime_part);
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

bool Controller::ManualSetSystemDatetime(Datetime *dt) {
    if (Controller::datetime_service == NULL) {
        return false;
    }
    return Controller::datetime_service->ManualSet(dt);
}

void Controller::GetSystemDatetime(Datetime *dt) {
    if (Controller::datetime_service == NULL) {
        return;
    }
    Controller::datetime_service->Get(dt);
}

void Controller::RestartSntp() {
    if (Controller::datetime_service == NULL) {
        return;
    }
    Controller::datetime_service->SntpStateChanged();
}

void Controller::StoreSystemDatetime() {
    if (Controller::datetime_service == NULL) {
        return;
    }
    Controller::datetime_service->StoreSystemDatetime();
}

void Controller::SetNetworkContinuation(LogicItemState state) {
    network_continuation = state;
}

LogicItemState Controller::GetNetworkContinuation() {
    return network_continuation;
}

void Controller::UpdateUIViewTop(int32_t view_top_index) {
    SAFETY_HOTRELOAD({
        hotreload->view_top_index = view_top_index;
        store_hotreload();
    });
    ESP_LOGD(TAG_Controller, "UpdateUIViewTop %d", view_top_index);
    Controller::WakeupProcessTask();
}

void Controller::UpdateUISelected(int32_t selected_network) {
    SAFETY_HOTRELOAD({
        hotreload->selected_network = selected_network;
        store_hotreload();
    });
    ESP_LOGD(TAG_Controller, "UpdateUISelected %d", selected_network);
}

int32_t Controller::GetLastUpdatedUISelected() {
    return hotreload->selected_network;
}

void Controller::DesignStart() {
    Controller::in_design = true;
    Controller::WakeupProcessTask();
    ESP_LOGI(TAG_Controller, "DesignStart");
}

void Controller::DesignEnd() {
    Controller::in_design = false;
    ESP_LOGI(TAG_Controller, "DesignEnd");
}

bool Controller::InDesign() {
    return Controller::in_design;
}