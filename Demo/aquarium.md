# Aquarium: light and filter pump

Program file: [aquarium.dat](aquarium.dat)

The controller turns the aquarium light on in the morning and off at night. In the
afternoon a photo sensor checks the room light: if it is too bright, the aquarium light
goes off, and it comes back when the room gets dark. The filter pump works in two periods
during the day. On weekends the day starts one hour later.

## Wiring

See sheet 3 of [Schematic_ESP8266_2025-04-25.pdf](../Docs/Schematic_ESP8266_2025-04-25.pdf).

| I/O | Connected to |
|-----|--------------|
| AI | photo sensor, a higher value means more light |
| DI | filter stop, used when you clean the aquarium |
| O1 | light |
| O2 | filter pump |

| Variable | Meaning |
|----------|---------|
| V1 | weekday, 1 = Monday ... 7 = Sunday |
| V2 | hour |
| V3 | minute |
| V4 | 1 = the light level is low, the aquarium light is needed |

## Schedule

| | Mon - Fri | Sat - Sun |
|---|---|---|
| Light on | 09:00 | 10:00 |
| Light off | 01:31 | 01:31 |
| Filter on | 11:00 - 18:00, 20:00 - 01:00 | 12:00 - 18:00, 20:00 - 01:00 |

## How it works

Set and Reset outputs act only at the moment their chain becomes active. Because of this,
a network like `V2>=20 -- Set(O2)` turns the pump on once at 20:00 and does not stop a
later Reset from turning it off. ContinuationOut takes the state of the last
ContinuationIn above it, so one ContinuationIn can feed several networks.

- **Networks 1-7.** Show the photo sensor, weekday, hour and minute on the screen and bind
  V1-V3 to the clock.
- **Networks 8-11, light level (V4).**
  - Before 13:00 the photo sensor is ignored and V4 is 1.
  - From 13:00, if AI stays at 110 or more for 3 minutes, V4 becomes 0. If AI stays at 80
    or less for 1 minute, V4 becomes 1 again.
  - The gap between 80 and 110 and the timers stop the light from blinking when the room
    light changes.
- **Networks 12-20, light (O1).**
  - When V4 is 1, the light goes on at 10:00 on weekends (12-15) and at 09:00 on weekdays
    (16-17).
  - On weekends, networks 14-15 also turn the light on between 00:05 and 01:30. If the
    device restarts after midnight, the light comes back on.
  - From 01:00 to 09:00 the light goes off at minute 31 of each hour (18-19).
  - When V4 becomes 0, the light goes off (20). When V4 becomes 1 again, networks 12-17
    turn it back on.
- **Networks 21-29, filter pump (O2).**
  - It goes on at 12:00 on weekends (21-22), at 11:00 on weekdays (23-24) and at 20:00
    every day (25). It goes off at 18:00 (27) and at 01:00 (29).
  - Network 28 turns it off again at 19:00.
  - Network 26 turns the pump on at 00:05. If there is no internet after power-on, the
    clock starts from 00:00, so the filter starts 5 minutes after power-on.
- **Networks 30-31.** Let you set the date and time on the device when there is no
  internet.
- **Network 32.** A signal on DI stops the filter. It starts again at the next time in the
  schedule.

## How to use

1. Wire the device as shown above.
2. Open the web interface and switch the controller to Stop.
3. Upload `aquarium.dat`, see [WEB_INTERFACE.md](../Docs/WEB_INTERFACE.md#program).
4. Check the program on the screen and switch to Run. This also saves it to flash.

The photo sensor limits (80 and 110) depend on your sensor and room. Look at the AI value on
the screen (network 1) in a dark and in a bright room, and change the comparators in
networks 9 and 10 if needed.

## Networks

Made with `Tools/ladder.py dump Demo/aquarium.dat`.

```
work_mode: Run, networks: 32
 1  Indicator[AI 0..255]
 2  Indicator[V1 0..255]
 3  Indicator[V2 0..255]
 4  Indicator[V3 0..255]
 5  DateTime[weekday -> V1]
 6  DateTime[hour -> V2]
 7  DateTime[minute -> V3]
 8  V2>=13 -- ContIn
 9  ContOut -- AI<=80 -- Timer[60s] -- Set(V4)
10  ContOut -- AI>=110 -- Timer[180s] -- Reset(V4)
11  V2<13 -- Set(V4)
12  V1>=6 -- V1<=7 -- NO[V4] -- ContIn
13  ContOut -- V2>=10 -- Set(O1)
14  ContOut -- V2==0 -- V3>=5 -- Set(O1)
15  ContOut -- V2==1 -- V3<=30 -- Set(O1)
16  V1>=1 -- V1<=5 -- NO[V4] -- ContIn
17  ContOut -- V2>=9 -- Set(O1)
18  V2>=1 -- V2<9 -- ContIn
19  ContOut -- V3>30 -- Reset(O1)
20  NC[V4] -- Reset(O1)
21  V1>=6 -- V1<=7 -- ContIn
22  ContOut -- V2>=12 -- V2<18 -- Set(O2)
23  V1>=1 -- V1<=5 -- ContIn
24  ContOut -- V2>=11 -- V2<18 -- Set(O2)
25  V2>=20 -- Set(O2)
26  V2==0 -- V3>=5 -- Set(O2)
27  V2==18 -- Reset(O2)
28  V2==19 -- Reset(O2)
29  V2==1 -- Reset(O2)
30  Settings[current_date]
31  Settings[current_time]
32  NO[DI] -- Reset(O2)
```

## Code for Ladder::InitialLoad()

Made with `Tools/ladder.py cpp Demo/aquarium.dat`.

<details>
<summary>Show code</summary>

```cpp
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
```

</details>
