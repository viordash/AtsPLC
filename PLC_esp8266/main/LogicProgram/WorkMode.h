#pragma once

#include <stdint.h>
#include <unistd.h>

enum WorkMode : uint8_t { Stop = 0, Run = 1 };

static inline bool ValidateWorkMode(WorkMode mode) {
    switch (mode) {
        case WorkMode::Stop:
        case WorkMode::Run:
            return true;

        default:
            break;
    }
    return false;
}

enum EffectiveWorkMode : uint8_t { ewm_Stop = 0, ewm_Run = 1, ewm_Debug = 2 };

static inline bool ValidateEffectiveWorkMode(EffectiveWorkMode mode) {
    switch (mode) {
        case EffectiveWorkMode::ewm_Stop:
        case EffectiveWorkMode::ewm_Run:
        case EffectiveWorkMode::ewm_Debug:
            return true;

        default:
            break;
    }
    return false;
}

static inline EffectiveWorkMode Convert2EffectiveWorkMode(WorkMode work_mode, bool enable_debug) {
    switch (work_mode) {
        case WorkMode::Stop:
            return EffectiveWorkMode::ewm_Stop;

        case WorkMode::Run:
            if (enable_debug) {
                return EffectiveWorkMode::ewm_Debug;
            }
            return EffectiveWorkMode::ewm_Run;
    }
    return EffectiveWorkMode::ewm_Stop;
}

static inline WorkMode Convert2WorkMode(EffectiveWorkMode mode) {
    switch (mode) {
        case EffectiveWorkMode::ewm_Run:
        case EffectiveWorkMode::ewm_Debug:
            return WorkMode::Run;

        case EffectiveWorkMode::ewm_Stop:
            return WorkMode::Stop;
    }
    return WorkMode::Stop;
}

static inline bool Convert2EnableDebug(EffectiveWorkMode mode) {
    return mode == EffectiveWorkMode::ewm_Debug;
}
