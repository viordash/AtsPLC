#include "CppUTestExt/MockSupport_c.h"

#include "main/Maintenance/smartconfig_service.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum SmartconfigStatus smartconfig_status() {
    return mock_c()->actualCall("smartconfig_status")->returnIntValueOrDefault(scs_Error);
}

void start_smartconfig() {
    mock_c()->actualCall("start_smartconfig");
}