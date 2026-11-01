#pragma once

#include "Display/Common.h"
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

class MonitorLogicElement {
  public:
    Point Render_start_point = {};
    bool DoAction_called = false;
    bool DoAction_result = false;
    bool Render_called = false;

    bool DoAction() {
        DoAction_called = true;
        return DoAction_result;
    }

    void Render(const Point *start_point) {
        Render_start_point = *start_point;
        Render_called = true;
    }
};