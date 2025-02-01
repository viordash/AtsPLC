#pragma once

#include "freertos/event_groups.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

enum SmartconfigStatus {
    scs_Start,
    scs_Started,
    scs_Disconnected,
    scs_GotIP,
    scs_ScanDone,
    scs_FoundChannel,
    scs_GotCreds,
    scs_Completed,
    scs_Error
};

void start_smartconfig();
enum SmartconfigStatus smartconfig_status();

#ifdef __cplusplus
}
#endif