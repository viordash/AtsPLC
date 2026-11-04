#include "LogicProgram/Ladder.h"
#include "LogicProgram/LogicProgram.h"
#include <stdio.h>
#include <stdlib.h>

void Ladder::InitialLoad() {
    // 1: Indicator[AI 0..255]
    {
        auto network = new Network(NetworkState::nsActive);
        auto indicator = new Indicator(MapIO::AI);
        indicator->SetLowScale(0);
        indicator->SetHighScale(255);
        indicator->SetDecimalPoint(0);
        indicator->UpdateScale();
        network->Append(indicator);
        Append(network);
    }
    // 2: Indicator[V1 0..255]
    {
        auto network = new Network(NetworkState::nsActive);
        auto indicator = new Indicator(MapIO::V1);
        indicator->SetLowScale(0);
        indicator->SetHighScale(255);
        indicator->SetDecimalPoint(0);
        indicator->UpdateScale();
        network->Append(indicator);
        Append(network);
    }
    // 3: Indicator[V2 0..255]
    {
        auto network = new Network(NetworkState::nsActive);
        auto indicator = new Indicator(MapIO::V2);
        indicator->SetLowScale(0);
        indicator->SetHighScale(255);
        indicator->SetDecimalPoint(0);
        indicator->UpdateScale();
        network->Append(indicator);
        Append(network);
    }
    // 4: Indicator[V3 0..255]
    {
        auto network = new Network(NetworkState::nsActive);
        auto indicator = new Indicator(MapIO::V3);
        indicator->SetLowScale(0);
        indicator->SetHighScale(255);
        indicator->SetDecimalPoint(0);
        indicator->UpdateScale();
        network->Append(indicator);
        Append(network);
    }
    // 5: DateTime[weekday -> V1]
    {
        auto network = new Network(NetworkState::nsActive);
        auto datetime = new DateTimeBinding(MapIO::V1);
        datetime->datetime_part = DatetimePart::t_weekday;
        network->Append(datetime);
        Append(network);
    }
    // 6: DateTime[hour -> V2]
    {
        auto network = new Network(NetworkState::nsActive);
        auto datetime = new DateTimeBinding(MapIO::V2);
        datetime->datetime_part = DatetimePart::t_hour;
        network->Append(datetime);
        Append(network);
    }
    // 7: DateTime[minute -> V3]
    {
        auto network = new Network(NetworkState::nsActive);
        auto datetime = new DateTimeBinding(MapIO::V3);
        datetime->datetime_part = DatetimePart::t_minute;
        network->Append(datetime);
        Append(network);
    }
    // 8: V2>=13 -- ContIn
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ComparatorGE(13, MapIO::V2));
        network->Append(new ContinuationIn());
        Append(network);
    }
    // 9: ContOut -- AI<=80 -- Timer[60s] -- Set(V4)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ContinuationOut());
        network->Append(new ComparatorLE(80, MapIO::AI));
        network->Append(new TimerSecs(60));
        network->Append(new SetOutput(MapIO::V4));
        Append(network);
    }
    // 10: ContOut -- AI>=110 -- Timer[180s] -- Reset(V4)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ContinuationOut());
        network->Append(new ComparatorGE(110, MapIO::AI));
        network->Append(new TimerSecs(180));
        network->Append(new ResetOutput(MapIO::V4));
        Append(network);
    }
    // 11: V2<13 -- Set(V4)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ComparatorLs(13, MapIO::V2));
        network->Append(new SetOutput(MapIO::V4));
        Append(network);
    }
    // 12: V1>=6 -- V1<=7 -- NO[V4] -- ContIn
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ComparatorGE(6, MapIO::V1));
        network->Append(new ComparatorLE(7, MapIO::V1));
        network->Append(new InputNO(MapIO::V4));
        network->Append(new ContinuationIn());
        Append(network);
    }
    // 13: ContOut -- V2>=10 -- Set(O1)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ContinuationOut());
        network->Append(new ComparatorGE(10, MapIO::V2));
        network->Append(new SetOutput(MapIO::O1));
        Append(network);
    }
    // 14: ContOut -- V2==0 -- V3>=5 -- Set(O1)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ContinuationOut());
        network->Append(new ComparatorEq(0, MapIO::V2));
        network->Append(new ComparatorGE(5, MapIO::V3));
        network->Append(new SetOutput(MapIO::O1));
        Append(network);
    }
    // 15: ContOut -- V2==1 -- V3<=30 -- Set(O1)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ContinuationOut());
        network->Append(new ComparatorEq(1, MapIO::V2));
        network->Append(new ComparatorLE(30, MapIO::V3));
        network->Append(new SetOutput(MapIO::O1));
        Append(network);
    }
    // 16: V1>=1 -- V1<=5 -- NO[V4] -- ContIn
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ComparatorGE(1, MapIO::V1));
        network->Append(new ComparatorLE(5, MapIO::V1));
        network->Append(new InputNO(MapIO::V4));
        network->Append(new ContinuationIn());
        Append(network);
    }
    // 17: ContOut -- V2>=9 -- Set(O1)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ContinuationOut());
        network->Append(new ComparatorGE(9, MapIO::V2));
        network->Append(new SetOutput(MapIO::O1));
        Append(network);
    }
    // 18: V2>=1 -- V2<9 -- ContIn
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ComparatorGE(1, MapIO::V2));
        network->Append(new ComparatorLs(9, MapIO::V2));
        network->Append(new ContinuationIn());
        Append(network);
    }
    // 19: ContOut -- V3>30 -- Reset(O1)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ContinuationOut());
        network->Append(new ComparatorGr(30, MapIO::V3));
        network->Append(new ResetOutput(MapIO::O1));
        Append(network);
    }
    // 20: NC[V4] -- Reset(O1)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new InputNC(MapIO::V4));
        network->Append(new ResetOutput(MapIO::O1));
        Append(network);
    }
    // 21: V1>=6 -- V1<=7 -- ContIn
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ComparatorGE(6, MapIO::V1));
        network->Append(new ComparatorLE(7, MapIO::V1));
        network->Append(new ContinuationIn());
        Append(network);
    }
    // 22: ContOut -- V2>=12 -- V2<18 -- Set(O2)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ContinuationOut());
        network->Append(new ComparatorGE(12, MapIO::V2));
        network->Append(new ComparatorLs(18, MapIO::V2));
        network->Append(new SetOutput(MapIO::O2));
        Append(network);
    }
    // 23: V1>=1 -- V1<=5 -- ContIn
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ComparatorGE(1, MapIO::V1));
        network->Append(new ComparatorLE(5, MapIO::V1));
        network->Append(new ContinuationIn());
        Append(network);
    }
    // 24: ContOut -- V2>=11 -- V2<18 -- Set(O2)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ContinuationOut());
        network->Append(new ComparatorGE(11, MapIO::V2));
        network->Append(new ComparatorLs(18, MapIO::V2));
        network->Append(new SetOutput(MapIO::O2));
        Append(network);
    }
    // 25: V2>=20 -- Set(O2)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ComparatorGE(20, MapIO::V2));
        network->Append(new SetOutput(MapIO::O2));
        Append(network);
    }
    // 26: V2==0 -- V3>=5 -- Set(O2)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ComparatorEq(0, MapIO::V2));
        network->Append(new ComparatorGE(5, MapIO::V3));
        network->Append(new SetOutput(MapIO::O2));
        Append(network);
    }
    // 27: V2==18 -- Reset(O2)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ComparatorEq(18, MapIO::V2));
        network->Append(new ResetOutput(MapIO::O2));
        Append(network);
    }
    // 28: V2==19 -- Reset(O2)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ComparatorEq(19, MapIO::V2));
        network->Append(new ResetOutput(MapIO::O2));
        Append(network);
    }
    // 29: V2==1 -- Reset(O2)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new ComparatorEq(1, MapIO::V2));
        network->Append(new ResetOutput(MapIO::O2));
        Append(network);
    }
    // 30: Settings[current_date]
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new SettingsElement(SettingsElement::Discriminator::t_current_date));
        Append(network);
    }
    // 31: Settings[current_time]
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new SettingsElement(SettingsElement::Discriminator::t_current_time));
        Append(network);
    }
    // 32: NO[DI] -- Reset(O2)
    {
        auto network = new Network(NetworkState::nsActive);
        network->Append(new InputNO(MapIO::DI));
        network->Append(new ResetOutput(MapIO::O2));
        Append(network);
    }
}
