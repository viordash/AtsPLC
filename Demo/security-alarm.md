# Security alarm with a phone as the key

Program file: [security-alarm.dat](security-alarm.dat)

The controller runs its own WiFi access point. When the owner's phone is connected to it,
the owner is at home. When nobody is connected and the door opens, the controller waits 30
seconds and then turns on the siren for 10 minutes. A red LED shows the guard state. The
door sensor line is checked too: a broken or shorted line is shown on the LED and, when
nobody is at home, starts the alarm.

## Wiring

See sheet 3 of [Schematic_ESP8266_2025-04-25.pdf](../Docs/Schematic_ESP8266_2025-04-25.pdf).

| I/O | Connected to |
|-----|--------------|
| AI | door sensor, see below |
| O1 | siren |
| O2 | red LED |

DI is not used. It is GPIO0, which also selects the boot mode of the ESP8266: if the door
contact pulls it to ground while the device restarts, the controller does not start.

### Door sensor

The door contact (normally open, closed when the door is closed) is a part of a resistor
divider on AI:

```
+3.3V ---- 2.5k ---+--- 390 ---- door contact ---- GND
                   |
                   +------------------------------ AI
                   |
                  820
                   |
                  GND
```

The ADC of the ESP8266 measures 0..1.0 V, so AI is about 255 × voltage.

| AI | Voltage | State |
|----|---------|-------|
| 0..29 | below 0.12 V | fault, for example the sensor line is shorted |
| 30..129 | 0.2 - 0.3 V | door closed |
| 130..229 | about 0.8 V | door open |
| 230..255 | above 0.9 V | fault, for example the 820 resistor is broken |

A cut line to the door contact looks like an open door. Resistor tolerance changes these
values. Look at AI on the screen (network 2) with the door open and closed, and move the
limits in networks 5-14 if needed. Keep each limit in the middle between two levels.

| Variable | Meaning |
|----------|---------|
| V1 | number of phones connected to the access point |
| V2 | guard state, see below |

| V2 | State |
|----|-------|
| 0 | quiet |
| 1..30 | the door was opened or the line failed while nobody was home, 30-second wait |
| 31..180 | alarm, the siren works |
| 181 | the alarm is over, the LED keeps the alarm until the owner comes back |

## LED

| LED (O2) | Meaning |
|----------|---------|
| off | the owner is at home, all is OK |
| short flash every 2 seconds (on 250 ms, off 2 s) | on guard: nobody at home, the door is closed |
| on 500 ms, off 500 ms | the guard was triggered |
| on 200 ms, off 200 ms | fault of the door sensor line while the owner is at home; when nobody is at home, a fault starts the alarm |

## How it works

Set, Reset and Inc outputs act only at the moment their chain becomes active.
ContinuationOut takes the state of the last ContinuationIn above it, so one ContinuationIn
can feed several networks. V2 is used as a counter: a SquareWave generator in front of an
Inc output adds 1 on each pulse, so the value of V2 also measures time.

- **Network 1.** Starts the access point `AtsPLC`. V1 is the number of connected phones
  that match the MAC mask.
- **Networks 2-4.** Show AI, V1 and V2 on the screen.
- **Networks 5-11, LED.**
  - Fault of the line while V2 is 0: fast blinking (5-8).
  - Door closed, no phone, V2 is 0: on guard, short flashes (9-10).
  - V2 is more than 0: the guard was triggered (11).
- **Networks 12-15, trigger.** When no phone is connected and V2 is 0, an open door (12-13)
  or a low fault (14-15) sets V2 to 1. Network 12 also catches the high fault, because AI
  is then above 130. `V1==0` stands first in networks 9, 12 and 14: V1 is bound to the
  access point, see [Reading a bound variable](../Docs/LADDER_ELEMENTS.md#reading-a-bound-variable).
- **Networks 16-17, 30-second wait.** While V2 is 1..30, it grows by 1 every second.
  Closing the door does not stop the wait.
- **Networks 18-20, alarm.** While V2 is 31..180, it grows by 1 every 4 seconds, which
  gives 10 minutes. The siren works 500 ms and pauses 500 ms (20). At 181 the siren stops,
  and the LED keeps blinking. The guard does not start again until the owner comes back.
- **Networks 21-24, the owner is back.** When a phone connects, V2, the siren and the LED
  are reset.
- **Networks 25-26.** SNTP servers, see Limitations.

## How to use

1. Wire the device as shown above.
2. Open the web interface and switch the controller to Stop.
3. Upload `security-alarm.dat`, see [WEB_INTERFACE.md](../Docs/WEB_INTERFACE.md#program).
4. In network 1, change the password `ats-PLC0`. Clear the SNTP servers in networks 25-26.
   Switch to Run.
5. Connect the phone to `AtsPLC` and turn on auto-connect. V1 on the screen becomes 1.
6. The MAC mask `*` lets any phone with the password in. To allow only your phone, find
   the MAC that the phone uses for `AtsPLC` in its WiFi settings (many phones use a random
   MAC for each network). Switch to Stop, enter this MAC as the mask in network 1, and
   switch back to Run.

## Limitations

- **The access point needs the radio alone.** When the device also has to connect to a WiFi
  station, the access point turns off for that time and V1 can show a wrong number. The
  device connects to a station when an SNTP server is set, so clear both SNTP servers
  (networks 25-26). Do not add WiFi bindings to this program.
- **No web interface.** The web interface needs a WiFi station connection, and without
  SNTP servers the device does not connect to a station. To use the web interface again,
  enter an SNTP server in network 25 and switch to Stop.
- **The phone must keep WiFi on.** If the phone drops the connection while you are at home,
  opening the door starts the 30-second wait. If it reconnects in time, nothing happens.
- **Close the door before you walk away.** If the phone loses the access point while the
  door is still open, the 30-second wait starts, and closing the door does not stop it.
- **AI is read once per ADC scan period** (`adc_scan_period_ms` in settings, 1 second by
  default), so the controller can see an open door up to 1 second later.
- **After power-on**, V1 is 0 until the phone connects again. If the door is open at this
  moment, the 30-second wait starts.

## Ladder diagram

Made with `Tools/ladder.py ascii Demo/security-alarm.dat`. The symbols are explained in
[README.md](README.md#ladder-diagram-symbols).

```
 1  ||--[WiFiAp V1 "AtsPLC" "ats-PLC0" "************"]----||

 2  ||--[Indicator AI 0..255]-----------------------------||

 3  ||--[Indicator V1 0..255]-----------------------------||

 4  ||--[Indicator V2 0..255]-----------------------------||

 5  ||--[AI >= 230]--[V2 == 0]---------------------------->>

                                                      O2
 6  >>--[SQW off 200ms on 200ms]---------------------( )--||

 7  ||--[AI < 30]--[V2 == 0]------------------------------>>

                                                      O2
 8  >>--[SQW off 200ms on 200ms]---------------------( )--||

 9  ||--[V1 == 0]--[AI >= 30]--[AI < 130]----------------->>

                                                      O2
10  >>--[V2 == 0]--[SQW off 2000ms on 250ms]---------( )--||

                                                      O2
11  ||--[V2 > 0]--[SQW off 500ms on 500ms]-----------( )--||

12  ||--[V1 == 0]--[AI >= 130]--[V2 == 0]----------------->>

                                                      V2
13  >>-----------------------------------------------(+)--||

14  ||--[V1 == 0]--[AI < 30]--[V2 == 0]------------------->>

                                                      V2
15  >>-----------------------------------------------(+)--||

16  ||--[V2 > 0]--[V2 <= 30]------------------------------>>

                                                      V2
17  >>--[SQW off 500ms on 500ms]---------------------(+)--||

18  ||--[V2 > 30]--[V2 <= 180]---------------------------->>

                                                      V2
19  >>--[SQW off 2000ms on 2000ms]-------------------(+)--||

                                                      O1
20  >>--[SQW off 500ms on 500ms]---------------------( )--||

21  ||--[V1 > 0]------------------------------------------>>

                                                      V2
22  >>-----------------------------------------------(R)--||

                                                      O1
23  >>-----------------------------------------------(R)--||

                                                      O2
24  >>-----------------------------------------------(R)--||

25  ||--[Settings datetime_sntp_server_primary]-----------||

26  ||--[Settings datetime_sntp_server_secondary]---------||
```

## Code for Ladder::InitialLoad()

Made with `Tools/ladder.py cpp Demo/security-alarm.dat`.

<details>
<summary>Show code</summary>

```cpp
void Ladder::InitialLoad() {
    //  1  ||--[WiFiAp V1 "AtsPLC" "ats-PLC0" "************"]----||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new WiFiApBinding(MapIO::V1, "AtsPLC", "ats-PLC0", "************"));
        Append(network);
    }
    //  2  ||--[Indicator AI 0..255]----||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        auto indicator = new Indicator(MapIO::AI);
        indicator->SetLowScale(0);
        indicator->SetHighScale(255);
        indicator->SetDecimalPoint(0);
        indicator->UpdateScale();
        network->Append(indicator);
        Append(network);
    }
    //  3  ||--[Indicator V1 0..255]----||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        auto indicator = new Indicator(MapIO::V1);
        indicator->SetLowScale(0);
        indicator->SetHighScale(255);
        indicator->SetDecimalPoint(0);
        indicator->UpdateScale();
        network->Append(indicator);
        Append(network);
    }
    //  4  ||--[Indicator V2 0..255]----||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        auto indicator = new Indicator(MapIO::V2);
        indicator->SetLowScale(0);
        indicator->SetHighScale(255);
        indicator->SetDecimalPoint(0);
        indicator->UpdateScale();
        network->Append(indicator);
        Append(network);
    }
    //  5  ||--[AI >= 230]--[V2 == 0]-->>
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorGE(230, MapIO::AI));
        network->Append(new ComparatorEq(0, MapIO::V2));
        network->Append(new ContinuationIn());
        Append(network);
    }
    //                                    O2
    //  6  >>--[SQW off 200ms on 200ms]--( )--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new SquareWaveGenerator(200, 200));
        network->Append(new DirectOutput(MapIO::O2));
        Append(network);
    }
    //  7  ||--[AI < 30]--[V2 == 0]-->>
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorLs(30, MapIO::AI));
        network->Append(new ComparatorEq(0, MapIO::V2));
        network->Append(new ContinuationIn());
        Append(network);
    }
    //                                    O2
    //  8  >>--[SQW off 200ms on 200ms]--( )--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new SquareWaveGenerator(200, 200));
        network->Append(new DirectOutput(MapIO::O2));
        Append(network);
    }
    //  9  ||--[V1 == 0]--[AI >= 30]--[AI < 130]-->>
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorEq(0, MapIO::V1));
        network->Append(new ComparatorGE(30, MapIO::AI));
        network->Append(new ComparatorLs(130, MapIO::AI));
        network->Append(new ContinuationIn());
        Append(network);
    }
    //                                                O2
    // 10  >>--[V2 == 0]--[SQW off 2000ms on 250ms]--( )--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new ComparatorEq(0, MapIO::V2));
        network->Append(new SquareWaveGenerator(2000, 250));
        network->Append(new DirectOutput(MapIO::O2));
        Append(network);
    }
    //                                              O2
    // 11  ||--[V2 > 0]--[SQW off 500ms on 500ms]--( )--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorGr(0, MapIO::V2));
        network->Append(new SquareWaveGenerator(500, 500));
        network->Append(new DirectOutput(MapIO::O2));
        Append(network);
    }
    // 12  ||--[V1 == 0]--[AI >= 130]--[V2 == 0]-->>
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorEq(0, MapIO::V1));
        network->Append(new ComparatorGE(130, MapIO::AI));
        network->Append(new ComparatorEq(0, MapIO::V2));
        network->Append(new ContinuationIn());
        Append(network);
    }
    //          V2
    // 13  >>--(+)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new IncOutput(MapIO::V2));
        Append(network);
    }
    // 14  ||--[V1 == 0]--[AI < 30]--[V2 == 0]-->>
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorEq(0, MapIO::V1));
        network->Append(new ComparatorLs(30, MapIO::AI));
        network->Append(new ComparatorEq(0, MapIO::V2));
        network->Append(new ContinuationIn());
        Append(network);
    }
    //          V2
    // 15  >>--(+)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new IncOutput(MapIO::V2));
        Append(network);
    }
    // 16  ||--[V2 > 0]--[V2 <= 30]-->>
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorGr(0, MapIO::V2));
        network->Append(new ComparatorLE(30, MapIO::V2));
        network->Append(new ContinuationIn());
        Append(network);
    }
    //                                    V2
    // 17  >>--[SQW off 500ms on 500ms]--(+)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new SquareWaveGenerator(500, 500));
        network->Append(new IncOutput(MapIO::V2));
        Append(network);
    }
    // 18  ||--[V2 > 30]--[V2 <= 180]-->>
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorGr(30, MapIO::V2));
        network->Append(new ComparatorLE(180, MapIO::V2));
        network->Append(new ContinuationIn());
        Append(network);
    }
    //                                      V2
    // 19  >>--[SQW off 2000ms on 2000ms]--(+)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new SquareWaveGenerator(2000, 2000));
        network->Append(new IncOutput(MapIO::V2));
        Append(network);
    }
    //                                    O1
    // 20  >>--[SQW off 500ms on 500ms]--( )--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new SquareWaveGenerator(500, 500));
        network->Append(new DirectOutput(MapIO::O1));
        Append(network);
    }
    // 21  ||--[V1 > 0]-->>
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorGr(0, MapIO::V1));
        network->Append(new ContinuationIn());
        Append(network);
    }
    //          V2
    // 22  >>--(R)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new ResetOutput(MapIO::V2));
        Append(network);
    }
    //          O1
    // 23  >>--(R)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new ResetOutput(MapIO::O1));
        Append(network);
    }
    //          O2
    // 24  >>--(R)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new ResetOutput(MapIO::O2));
        Append(network);
    }
    // 25  ||--[Settings datetime_sntp_server_primary]----||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new SettingsElement(SettingsElement::Discriminator::t_datetime_sntp_server_primary));
        Append(network);
    }
    // 26  ||--[Settings datetime_sntp_server_secondary]----||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new SettingsElement(SettingsElement::Discriminator::t_datetime_sntp_server_secondary));
        Append(network);
    }
}
```

</details>
