#include "LogicProgram/Ladder.h"
#include "LogicProgram/LogicProgram.h"
#include <stdio.h>
#include <stdlib.h>

void Ladder::InitialLoad() {
    {
        auto network = new Network();
        network->Append(new InputNO(MapIO::DI));
        network->Append(new IncOutput(MapIO::V1));
        Append(network);
    }
    {
        auto network = new Network();
        network->Append(new Indicator(MapIO::V1));
        Append(network);
    }

    {
        auto network0 = new Network();
        network0->Append(new ComparatorEq(1, MapIO::V1));
        network0->Append(new ContinuationIn());
        Append(network0);

        auto network1 = new Network();
        network1->Append(new ContinuationOut());
        network1->Append(new WiFiStaBinding(MapIO::V2));
        Append(network1);
    }
    {
        auto network0 = new Network();
        network0->Append(new ComparatorEq(4, MapIO::V1));
        network0->Append(new ContinuationIn());
        Append(network0);

        auto network1 = new Network();
        network1->Append(new ContinuationOut());
        network1->Append(new WiFiStaBinding(MapIO::V2));
        Append(network1);
    }

    {
        auto network0 = new Network();
        network0->Append(new ComparatorEq(2, MapIO::V1));
        network0->Append(new ContinuationIn());
        Append(network0);

        auto network1 = new Network();
        network1->Append(new ContinuationOut());
        network1->Append(new WiFiApBinding(MapIO::V3, "APiyQd3ss4d", "passwordyqDQ", "*"));
        Append(network1);
    }
    {
        auto network0 = new Network();
        network0->Append(new ComparatorEq(4, MapIO::V1));
        network0->Append(new ContinuationIn());
        Append(network0);

        auto network1 = new Network();
        network1->Append(new ContinuationOut());
        network1->Append(new WiFiApBinding(MapIO::V3, "APiyQd3ss4d", "passwordyqDQ", "*"));
        Append(network1);
    }

    {
        auto network0 = new Network();
        network0->Append(new ComparatorEq(3, MapIO::V1));
        network0->Append(new ContinuationIn());
        Append(network0);

        auto network1 = new Network();
        network1->Append(new ContinuationOut());
        network1->Append(new WiFiBinding(MapIO::V4, "iyQd3ss4d"));
        Append(network1);
    }
    {
        auto network0 = new Network();
        network0->Append(new ComparatorEq(4, MapIO::V1));
        network0->Append(new ContinuationIn());
        Append(network0);

        auto network1 = new Network();
        network1->Append(new ContinuationOut());
        network1->Append(new WiFiBinding(MapIO::V4, "iyQd3ss4d"));
        Append(network1);
    }

    {
        auto network0 = new Network();
        network0->Append(new ComparatorEq(5, MapIO::V1));
        network0->Append(new ResetOutput(MapIO::V1));
        Append(network0);
    }
    {
        auto network = new Network();
        network->Append(new ComparatorGE(10, MapIO::V2));
        network->Append(new DirectOutput(MapIO::O1));
        Append(network);
    }
    {
        auto network = new Network();
        network->Append(new ComparatorGE(40, MapIO::V3));
        network->Append(new DirectOutput(MapIO::O1));
        Append(network);
    }
    {
        auto network = new Network();
        network->Append(new ComparatorGE(50, MapIO::V4));
        network->Append(new DirectOutput(MapIO::O1));
        Append(network);
    }
}
