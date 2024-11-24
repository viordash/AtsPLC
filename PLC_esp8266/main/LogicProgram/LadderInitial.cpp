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
    auto incomeRail2 = new Network(LogicItemState::lisActive);
    Append(incomeRail2);
    auto incomeRail3 = new Network(LogicItemState::lisActive);
    Append(incomeRail3);

    incomeRail0->Append(new InputNC(MapIO::DI));
    incomeRail0->Append(new InputNC(MapIO::V1));
    incomeRail0->Append(new TimerMSecs(500));
    incomeRail0->Append(new SetOutput(MapIO::V1));

    incomeRail1->Append(new InputNC(MapIO::DI));
    incomeRail1->Append(new InputNO(MapIO::V1));
    incomeRail1->Append(new TimerMSecs(500));
    incomeRail1->Append(new ResetOutput(MapIO::V1));

    incomeRail2->Append(new InputNO(MapIO::V1));
    incomeRail2->Append(new DirectOutput(MapIO::O1));

    incomeRail3->Append(new InputNO(MapIO::DI));
    incomeRail3->Append(new TimerSecs(3));
    incomeRail3->Append(new DirectOutput(MapIO::O2));

    auto incomeRail4 = new Network(LogicItemState::lisActive);
    incomeRail4->Append(new InputNO(MapIO::V1));
    incomeRail4->Append(new IncOutput(MapIO::V3));
    Append(incomeRail4);

    auto incomeRail5 = new Network(LogicItemState::lisActive);
    incomeRail5->Append(new ComparatorGE(254, MapIO::V3));
    incomeRail5->Append(new TimerSecs(3));
    incomeRail5->Append(new ResetOutput(MapIO::V3));
    Append(incomeRail5);
}
