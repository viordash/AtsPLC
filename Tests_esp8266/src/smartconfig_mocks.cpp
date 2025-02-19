#include "CppUTestExt/MockSupport.h"

#include "main/Maintenance/smartconfig_service.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum SmartconfigStatus smartconfig_status() {
    return (SmartconfigStatus)mock().actualCall("smartconfig_status").returnIntValueOrDefault(scs_Error);
}

void start_smartconfig() {
    mock().actualCall("start_smartconfig");
}

void finish_smartconfig() {
    mock().actualCall("finish_smartconfig");
}