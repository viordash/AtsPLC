#pragma once

#include "LogicProgram/LogicItemState.h"

struct Continuation
{
    LogicItemState state;
    bool state_changed;
};
