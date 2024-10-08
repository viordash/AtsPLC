#pragma once

#include <stdint.h>
#include <unistd.h>

enum TvElementType : uint8_t {
    et_Undef = 0,
    et_InputNC,
    et_InputNO,
    et_TimerSecs,
    et_TimerMSecs,
    et_ComparatorEq,
    et_ComparatorGE,
    et_ComparatorGr,
    et_ComparatorLE,
    et_ComparatorLs,
    et_DirectOutput,
    et_SetOutput,
    et_ResetOutput,
    et_IncOutput,
    et_DecOutput
};

struct __attribute__((packed)) TvElement {
    TvElementType type;
    // uint8_t value[];
};
