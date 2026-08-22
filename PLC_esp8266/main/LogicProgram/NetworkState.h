#pragma once

#include "LogicProgram/LogicItemState.h"
#include <stdint.h>
#include <unistd.h>

enum NetworkState : uint8_t {
    nsPassive = 0,
    nsActive,
    nsStopFromPassive,
    nsStopFromActive
};

static inline bool ValidateNetworkState(NetworkState state) {
    switch (state) {
        case NetworkState::nsPassive:
        case NetworkState::nsActive:
        case NetworkState::nsStopFromPassive:
        case NetworkState::nsStopFromActive:
            return true;

        default:
            break;
    }
    return false;
}

static inline LogicItemState Convert2LogicItemState(NetworkState state) {
    switch (state) {
        case NetworkState::nsPassive:
            return LogicItemState::lisPassive;

        case NetworkState::nsActive:
            return LogicItemState::lisActive;

        case NetworkState::nsStopFromPassive:
            return LogicItemState::lisStop;

        case NetworkState::nsStopFromActive:
            return LogicItemState::lisStop;
    }
    return LogicItemState::lisPassive;
}
