#include "LogicProgram/Ladder.h"
#include "LogicProgram/LogicProgram.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Ladder::InitialLoad() {
    auto incomeRail0 = new Network(LogicItemState::lisActive);
    Append(incomeRail0);
    auto incomeRail1 = new Network(LogicItemState::lisActive);
    Append(incomeRail1);

    incomeRail0->Append(new InputNO(MapIO::DI));
    incomeRail0->Append(new InputNC(MapIO::V1));
    incomeRail0->Append(new TimerSecs(1));
    incomeRail0->Append(new SetOutput(MapIO::V1));

    incomeRail1->Append(new InputNO(MapIO::DI));
    incomeRail1->Append(new InputNO(MapIO::V1));
    incomeRail1->Append(new TimerSecs(1));
    incomeRail1->Append(new ResetOutput(MapIO::V1));
}
