#pragma once

#include <stdint.h>
#include <unistd.h>

enum TvElementType : uint8_t {
    et_Undef = 0,
    et_Wire,
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
    et_DecOutput,
    et_Indicator
};

struct __attribute__((packed)) TvElement {
    TvElementType type;
    // uint8_t value[];
};

static inline bool IsInputElement(TvElementType element_type) {
    switch (element_type) {
        case et_InputNC:
            return true;
        case et_InputNO:
            return true;
        case et_TimerSecs:
            return true;
        case et_TimerMSecs:
            return true;
        case et_ComparatorEq:
            return true;
        case et_ComparatorGE:
            return true;
        case et_ComparatorGr:
            return true;
        case et_ComparatorLE:
            return true;
        case et_ComparatorLs:
            return true;
        default:
            return false;
    }
}

static inline bool IsOutputElement(TvElementType element_type) {
    switch (element_type) {
        case et_DirectOutput:
            return true;
        case et_SetOutput:
            return true;
        case et_ResetOutput:
            return true;
        case et_IncOutput:
            return true;
        case et_DecOutput:
            return true;
        default:
            return false;
    }
}