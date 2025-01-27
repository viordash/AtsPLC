#include "CppUTestExt/MockSupport_c.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

bool smartconfig_is_runned() {
    return mock_c()->actualCall("smartconfig_is_runned")->returnBoolValueOrDefault(false);
}

void start_smartconfig() {
    mock_c()->actualCall("start_smartconfig");
}