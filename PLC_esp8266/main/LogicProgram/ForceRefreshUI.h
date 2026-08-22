#pragma once

#include <stdint.h>

enum ForceRefreshUI : uint8_t {
    fru_None = 0,
    fru_WorkMode = 1 << 0,
};
