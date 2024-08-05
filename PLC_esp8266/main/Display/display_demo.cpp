#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Display/Common.h"
#include "LogicProgram/LogicProgram.h"
#include "display.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void display_demo_0(bool active) {
    IncomeRail incomeRail0(0);
    InputNO input1(MapIO::DI, incomeRail0);
    ComparatorGE comparator1(5, MapIO::AI, input1);
    ComparatorEQ comparator2(42, MapIO::V1, comparator1);
    DirectOutput directOutput0(MapIO::O1, comparator2);

    if (active) {
        input1.DoAction();
        comparator1.DoAction();
        comparator2.DoAction();
        directOutput0.DoAction();
    }

    IncomeRail incomeRail1(1);
    TimerSecs timerSecs1(2, incomeRail1);
    TimerSecs timerSecs2(10, timerSecs1);
    TimerMSecs timerMSecs3(100000, timerSecs2);

    if (active) {
        timerSecs1.DoAction();
        timerSecs2.DoAction();
        timerMSecs3.DoAction();
    }

    begin_render();
    incomeRail0.Render(get_display_buffer());
    input1.Render(get_display_buffer());
    comparator1.Render(get_display_buffer());
    comparator2.Render(get_display_buffer());
    directOutput0.Render(get_display_buffer());

    incomeRail1.Render(get_display_buffer());
    timerSecs1.Render(get_display_buffer());
    timerSecs2.Render(get_display_buffer());
    timerMSecs3.Render(get_display_buffer());
    end_render();

    for (size_t i = 0; i < 10; i++) {   
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        begin_render();

        incomeRail0.Render(get_display_buffer());
        input1.Render(get_display_buffer());
        comparator1.Render(get_display_buffer());
        comparator2.Render(get_display_buffer());
        directOutput0.Render(get_display_buffer());
        incomeRail1.Render(get_display_buffer());
        timerSecs1.Render(get_display_buffer());
        timerSecs2.Render(get_display_buffer());
        timerMSecs3.Render(get_display_buffer());
        end_render();
    }
}
