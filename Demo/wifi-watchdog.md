# WiFi watchdog: restart a router that stops working

Program file: [wifi-watchdog.dat](wifi-watchdog.dat)

The controller powers a WiFi router and listens to its network. When the signal of this
network stays weak for 1 minute, or is lost for 30 seconds, the controller cuts the power
of the router for 5 seconds. After 3 restarts in a row that do not help, the controller
stops and waits until the signal comes back. A red LED shows the state.

## Wiring

See sheet 3 of [Schematic_ESP8266_2025-04-25.pdf](../Docs/Schematic_ESP8266_2025-04-25.pdf).

| I/O | Connected to |
|-----|--------------|
| O1 | solid-state relay in the power line of the router, the router works while O1 is on |
| O2 | red LED |

Power the controller itself from a line that the relay does not switch.

| Variable | Meaning |
|----------|---------|
| V1 | signal level of the router network, 0 = the network is not found |
| V2 | state, see below |
| V3 | number of restarts in a row |

V1 is the signal level scaled to 0..255: -120 dBm gives 0 and -26 dBm gives 255 (the
scanner `min_rssi` and `max_rssi` settings). The limit 100 is about -83 dBm.

| V2 | State |
|----|-------|
| 0 | the router starts, the controller waits 2 minutes |
| 1 | the controller watches the signal |
| 2 | the power of the router is cut |

## LED

| LED (O2) | Meaning |
|----------|---------|
| on 500 ms, off 500 ms | the router starts |
| short flash every 5 seconds (on 250 ms, off 4750 ms) | the controller watches the signal, all is OK |
| on 200 ms, off 200 ms | 3 restarts in a row did not help, the controller waits for the signal |
| always on | the power of the router is cut |

## How it works

Set, Reset and Inc outputs act only at the moment their chain becomes active.
ContinuationOut takes the state of the last ContinuationIn above it.

- **Network 1.** Scans the air for the router network and puts its signal level into V1.
- **Networks 2-4.** Show V1, V2 and V3 on the screen.
- **Networks 5-6, the router starts.** While V2 is 0, the router gets power (5). After 2
  minutes V2 becomes 1 (6). The wait gives the router time to start, also when the
  controller and the router get power at the same time.
- **Networks 7-10, watching.** While V2 is 1 and V3 is less than 3, V2 becomes 2 when V1
  stays below 100 for 1 minute (7-8) or stays at 0 for 30 seconds (9-10). The delays keep a
  short noise on the air from restarting the router. The comparators
  on V1 stand first in networks 7 and 9: V1 is bound to the scanner, see
  [Reading a bound variable](../Docs/LADDER_ELEMENTS.md#reading-a-bound-variable).
- **Networks 11-13, restart.** When V2 becomes 2, the power is cut (11) and V3 grows by 1
  (12). After 5 seconds V2 becomes 0 (13), and the router starts again.
- **Network 14.** When V1 stays at 100 or more for 1 minute, V3 becomes 0. So V3 counts
  only the restarts after which the router did not come back.
- **Networks 15-20, LED.** One network for each state.
- **Networks 21-22.** SNTP servers, see Limitations.

If the router does not come back, its signal is already lost for more than 30 seconds when
the 2-minute wait ends, so the next restart starts at once. The controller makes 3
restarts about 2 minutes apart and then stops. When somebody repairs the router and its
signal is good for 1 minute, V3 becomes 0 and the watching works again.

## How to use

1. Wire the device as shown above.
2. Open the web interface and switch the controller to Stop.
3. Upload `wifi-watchdog.dat`, see [WEB_INTERFACE.md](../Docs/WEB_INTERFACE.md#program).
4. In network 1, change `AtsPLC` to the name of your router network. Clear the SNTP
   servers in networks 21-22. Switch to Run.
5. Look at V1 on the screen (network 2). Choose the limit for a weak signal and change the
   comparators in networks 7 and 14 if needed.

The default network name `AtsPLC` is the access point of the
[security-alarm](security-alarm.md) demo. A second controller with that program can play
the router when you test this one.

## Limitations

- **The scanner shares the radio with the WiFi station.** When no other WiFi request
  waits, the device connects to the WiFi station from the settings, see
  [WIFI.md](../Docs/WIFI.md#station-wifiservice_stationcpp). A new scan waits until the
  station has worked for `min_worktime_ms` (10 seconds by default), so V1 is updated about
  every 10 seconds. With an empty station name in the settings the scanner has the radio
  alone, but then the web interface does not work. Do not add other WiFi bindings to this
  program.
- **Clear the SNTP servers** (networks 21-22). With an SNTP server set, the device also
  asks for the station connection to get the time, and the scans wait for it more often.
- **The web interface** works in Stop. In Run it works only while the station is connected
  between two scans.
- **A lost signal is seen 30-37 seconds later.** V1 becomes 0 only after a full scan that
  does not find the network: 14 channels × `per_channel_scan_time_ms` (500 ms by default),
  about 7 seconds. Then the 30-second timer starts.
- **The router has no power while the controller is in Stop or starts.** The solid-state
  relay is open while O1 is off.

## Ladder diagram

Made with `Tools/ladder.py ascii Demo/wifi-watchdog.dat`. The symbols are explained in
[README.md](README.md#ladder-diagram-symbols).

```
 1  ||--[WiFi V1 "AtsPLC"]---------------------------||

 2  ||--[Indicator V1 0..255]------------------------||

 3  ||--[Indicator V2 0..255]------------------------||

 4  ||--[Indicator V3 0..255]------------------------||

                                                 O1
 5  ||--[V2 == 0]-------------------------------(S)--||

                                                 V2
 6  ||--[V2 == 0]--[TON 120s]-------------------(+)--||

 7  ||--[V1 < 100]--[TON 60s]------------------------>>

                                                 V2
 8  >>--[V2 == 1]--[V3 < 3]---------------------(+)--||

 9  ||--[V1 == 0]--[TON 30s]------------------------->>

                                                 V2
10  >>--[V2 == 1]--[V3 < 3]---------------------(+)--||

                                                 O1
11  ||--[V2 == 2]-------------------------------(R)--||

                                                 V3
12  ||--[V2 == 2]-------------------------------(+)--||

                                                 V2
13  ||--[V2 == 2]--[TON 5s]---------------------(R)--||

                                                 V3
14  ||--[V1 >= 100]--[TON 60s]------------------(R)--||

                                                 O2
15  ||--[V2 == 0]--[SQW off 500ms on 500ms]-----( )--||

16  ||--[V2 == 1]--[V3 < 3]-------------------------->>

                                                 O2
17  >>--[SQW off 4750ms on 250ms]---------------( )--||

18  ||--[V2 == 1]--[V3 >= 3]------------------------->>

                                                 O2
19  >>--[SQW off 200ms on 200ms]----------------( )--||

                                                 O2
20  ||--[V2 == 2]-------------------------------( )--||

21  ||--[Settings datetime_sntp_server_primary]------||

22  ||--[Settings datetime_sntp_server_secondary]----||
```

## Code for Ladder::InitialLoad()

Made with `Tools/ladder.py cpp Demo/wifi-watchdog.dat`.

<details>
<summary>Show code</summary>

```cpp
void Ladder::InitialLoad() {
    //  1  ||--[WiFi V1 "AtsPLC"]----||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new WiFiBinding(MapIO::V1, "AtsPLC"));
        Append(network);
    }
    //  2  ||--[Indicator V1 0..255]----||
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
    //  3  ||--[Indicator V2 0..255]----||
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
    //  4  ||--[Indicator V3 0..255]----||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        auto indicator = new Indicator(MapIO::V3);
        indicator->SetLowScale(0);
        indicator->SetHighScale(255);
        indicator->SetDecimalPoint(0);
        indicator->UpdateScale();
        network->Append(indicator);
        Append(network);
    }
    //                     O1
    //  5  ||--[V2 == 0]--(S)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorEq(0, MapIO::V2));
        network->Append(new SetOutput(MapIO::O1));
        Append(network);
    }
    //                                 V2
    //  6  ||--[V2 == 0]--[TON 120s]--(+)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorEq(0, MapIO::V2));
        network->Append(new TimerSecs(120));
        network->Append(new IncOutput(MapIO::V2));
        Append(network);
    }
    //  7  ||--[V1 < 100]--[TON 60s]-->>
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorLs(100, MapIO::V1));
        network->Append(new TimerSecs(60));
        network->Append(new ContinuationIn());
        Append(network);
    }
    //                               V2
    //  8  >>--[V2 == 1]--[V3 < 3]--(+)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new ComparatorEq(1, MapIO::V2));
        network->Append(new ComparatorLs(3, MapIO::V3));
        network->Append(new IncOutput(MapIO::V2));
        Append(network);
    }
    //  9  ||--[V1 == 0]--[TON 30s]-->>
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorEq(0, MapIO::V1));
        network->Append(new TimerSecs(30));
        network->Append(new ContinuationIn());
        Append(network);
    }
    //                               V2
    // 10  >>--[V2 == 1]--[V3 < 3]--(+)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new ComparatorEq(1, MapIO::V2));
        network->Append(new ComparatorLs(3, MapIO::V3));
        network->Append(new IncOutput(MapIO::V2));
        Append(network);
    }
    //                     O1
    // 11  ||--[V2 == 2]--(R)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorEq(2, MapIO::V2));
        network->Append(new ResetOutput(MapIO::O1));
        Append(network);
    }
    //                     V3
    // 12  ||--[V2 == 2]--(+)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorEq(2, MapIO::V2));
        network->Append(new IncOutput(MapIO::V3));
        Append(network);
    }
    //                               V2
    // 13  ||--[V2 == 2]--[TON 5s]--(R)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorEq(2, MapIO::V2));
        network->Append(new TimerSecs(5));
        network->Append(new ResetOutput(MapIO::V2));
        Append(network);
    }
    //                                  V3
    // 14  ||--[V1 >= 100]--[TON 60s]--(R)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorGE(100, MapIO::V1));
        network->Append(new TimerSecs(60));
        network->Append(new ResetOutput(MapIO::V3));
        Append(network);
    }
    //                                               O2
    // 15  ||--[V2 == 0]--[SQW off 500ms on 500ms]--( )--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorEq(0, MapIO::V2));
        network->Append(new SquareWaveGenerator(500, 500));
        network->Append(new DirectOutput(MapIO::O2));
        Append(network);
    }
    // 16  ||--[V2 == 1]--[V3 < 3]-->>
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorEq(1, MapIO::V2));
        network->Append(new ComparatorLs(3, MapIO::V3));
        network->Append(new ContinuationIn());
        Append(network);
    }
    //                                     O2
    // 17  >>--[SQW off 4750ms on 250ms]--( )--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new SquareWaveGenerator(4750, 250));
        network->Append(new DirectOutput(MapIO::O2));
        Append(network);
    }
    // 18  ||--[V2 == 1]--[V3 >= 3]-->>
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorEq(1, MapIO::V2));
        network->Append(new ComparatorGE(3, MapIO::V3));
        network->Append(new ContinuationIn());
        Append(network);
    }
    //                                    O2
    // 19  >>--[SQW off 200ms on 200ms]--( )--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new SquareWaveGenerator(200, 200));
        network->Append(new DirectOutput(MapIO::O2));
        Append(network);
    }
    //                     O2
    // 20  ||--[V2 == 2]--( )--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorEq(2, MapIO::V2));
        network->Append(new DirectOutput(MapIO::O2));
        Append(network);
    }
    // 21  ||--[Settings datetime_sntp_server_primary]----||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new SettingsElement(SettingsElement::Discriminator::t_datetime_sntp_server_primary));
        Append(network);
    }
    // 22  ||--[Settings datetime_sntp_server_secondary]----||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new SettingsElement(SettingsElement::Discriminator::t_datetime_sntp_server_secondary));
        Append(network);
    }
}
```

</details>
