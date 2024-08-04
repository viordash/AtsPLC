#include "Display/Common.h"
#include "LogicProgram/LogicProgram.h"
#include "display.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void display_demo_0(bool active) {
    begin_render();

    IncomeRail incomeRail0(0);
    InputNO input1(MapIO::DI, incomeRail0);
    ComparatorGE comparator1(5, MapIO::AI, input1);
    ComparatorGE comparator2(42, MapIO::V1, comparator1);
    ComparatorGE comparator3(1000, MapIO::V1, comparator2);

    if (active) {
        input1.DoAction();
        comparator1.DoAction();
        comparator2.DoAction();
        comparator3.DoAction();
    }
    incomeRail0.Render(get_display_buffer());
    input1.Render(get_display_buffer());
    comparator1.Render(get_display_buffer());
    comparator2.Render(get_display_buffer());
    comparator3.Render(get_display_buffer());

    IncomeRail incomeRail1(1);
    TimerSecs timerSecs1(1, incomeRail1);
    TimerSecs timerSecs2(942, timerSecs1);
    TimerSecs timerSecs3(52349, timerSecs2);

    if (active) {
        timerSecs1.DoAction();
        timerSecs2.DoAction();
        timerSecs3.DoAction();
    }

    incomeRail1.Render(get_display_buffer());
    timerSecs1.Render(get_display_buffer());
    timerSecs2.Render(get_display_buffer());
    timerSecs3.Render(get_display_buffer());
    end_render();
}
