#pragma once

#include <stdint.h>
#include <unistd.h>

enum LogicItemState : uint8_t { lisPassive = 0, lisActive, lisStop };

static inline bool ValidateLogicItemState(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisPassive:
        case LogicItemState::lisActive:
        case LogicItemState::lisStop:
            return true;

        default:
            break;
    }
    return false;
}
