#include "Display/Common.h"
#include "Display/DisplayItemBase.h"
#include "LogicProgram/InputNO.h"
#include "LogicProgram/LogicItemBase.h"
#include "LogicProgram/MapIO.h"
#include "display.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputNO input1(MapIO::DI, { 60, 32 });

void display_demo_0() {
    begin_render();
    input1.Render(get_display_buffer());
    end_render();
}

