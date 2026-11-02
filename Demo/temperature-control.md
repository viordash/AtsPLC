# Temperature control: heater and fan

Program file: [temperature-control.dat](temperature-control.dat)

The controller keeps the temperature in a box, for example a box with equipment, between
the setpoints. When it is too cold, a heater works. When it is too hot, a fan works. The
temperature sensor is a thermistor. A broken or shorted sensor turns both outputs off at
once.

## Wiring

See sheet 3 of [Schematic_ESP8266_2025-04-25.pdf](../Docs/Schematic_ESP8266_2025-04-25.pdf).

| I/O | Connected to |
|-----|--------------|
| AI | thermistor, see below |
| O1 | heater |
| O2 | fan |

### Temperature sensor

The sensor is an NTC thermistor of 10 kOhm at 25 °C with B = 3950, a common type. Its
resistance falls when the temperature grows: 12.5 kOhm at 20 °C, 8.0 kOhm at 30 °C,
5.3 kOhm at 40 °C.

The ADC of the ESP8266 measures 0..1.0 V, and A0 goes to the ADC without a divider on the
board. So the circuit must keep AI at 1 V or less, also when the sensor breaks:

```
IN+ (+3.3V) ---- R1 24k ----+---------------+
                            |               |
AI -------------------------+          thermistor
                            |            NTC 10k
                          R2 10k            |
                            |               |
IN- (GND) ------------------+---------------+
```

R2 is in parallel with the thermistor. Without R2, a cut thermistor wire would put 3.3 V on
AI. With R2, a cut wire gives 0.97 V (AI 248), and a shorted thermistor gives 0 V. Both are
far from the normal values, so the program can see them. The price of R2 is a smaller range:
the 20..40 °C range takes 52 steps of AI, about 0.4 °C per step. The current through the
thermistor is about 60 µA, so it does not heat itself.

| °C | 18 | 20 | 22 | 24 | 26 | 28 | 30 | 32 | 34 | 36 | 38 | 40 | 42 |
|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
| AI | 164 | 158 | 153 | 148 | 142 | 137 | 132 | 126 | 121 | 116 | 111 | 106 | 101 |

A sensor with other values needs other setpoints. Use these formulas, where T is in kelvin:

```
R(T)  = R25 × exp(B × (1/T - 1/298.15))
V(T)  = 3.3 × (R2 ∥ R(T)) / (R1 + R2 ∥ R(T))
AI(T) ≈ 255.75 × V(T)
```

## Setpoints

| Output | On | Off |
|--------|----|-----|
| heater (O1) | 22 °C and below (AI ≥ 153) | 25 °C and above (AI ≤ 145) |
| fan (O2) | 35 °C and above (AI ≤ 119) | 32 °C and below (AI ≥ 126) |

The thermistor is an NTC, so a higher temperature gives a lower AI. Between 25 and 32 °C
both outputs are off, so the heater and the fan never work together.

| AI | State |
|----|-------|
| 240..255 | fault, the sensor wire is cut |
| 10..239 | normal |
| 0..9 | fault, the sensor is shorted |

## How it works

Set and Reset outputs act only at the moment their chain becomes active. A timer passes the
signal when its input has been active for the whole delay, and any change of the input
starts the count again. ContinuationOut takes the state of the last ContinuationIn above it.

All networks with setpoints are at the top of the program.

- **Networks 1-2, heater on.** When AI is 153..239 for 5 seconds, the heater goes on.
- **Network 3, heater off.** When AI is 145 or less for 5 seconds, the heater goes off.
- **Networks 4-5, fan on.** When AI is 10..119 for 5 seconds, the fan goes on.
- **Network 6, fan off.** When AI is 126 or more for 5 seconds, the fan goes off.
- **Networks 7-8, sensor fault.** A cut wire turns the heater off (7), and a shorted
  sensor turns the fan off (8), at once and without a delay. The other output goes off in
  networks 3 and 6, after their 5-second timers. When the sensor is fine again, the chains
  of networks 1 and 4 start their timers again and turn the outputs back on if needed.
- **Network 9.** Shows the temperature in °C. The Indicator scale is a straight line from
  80 °C at AI 0 to -16.8 °C at AI 255. Between 20 and 40 °C it differs from the real
  thermistor curve by 0.2 °C or less.
- **Network 10.** Shows the raw AI value, to check the sensor and to choose setpoints.

The two thresholds of each output and the 5-second timers keep the outputs from switching
on and off quickly when the temperature is near a setpoint: the noise of the ADC only
starts the timer again.

## How to use

1. Wire the device and the sensor as shown above.
2. Open the web interface and switch the controller to Stop.
3. Upload `temperature-control.dat`, see
   [WEB_INTERFACE.md](../Docs/WEB_INTERFACE.md#program).
4. Put the sensor next to a thermometer and compare it with the temperature on the screen
   (network 9). If the difference is large, check the thermistor type and the resistors.
5. Change the setpoints in networks 1-6 if needed, using the table above. Switch to Run.

## Limitations

- **AI is read once per ADC scan period** (`adc_scan_period_ms` in settings, 1 second by
  default), so a 5-second timer sees about 5 readings.
- **The temperature on the screen is approximate** outside 20..40 °C, because the
  Indicator scale is a straight line and the thermistor curve is not.
- **Resistor and thermistor tolerance** shift the values in the table. A 1% resistor and a
  1% thermistor give an error of about 0.5 °C.
- **For a heater or a fan that does not like frequent starts**, for example one with a
  compressor, set the timers in networks 2, 3, 5 and 6 to 30..60 seconds.
- **Both outputs are off while the controller is in Stop.**

## Ladder diagram

Made with `Tools/ladder.py ascii Demo/temperature-control.dat`. The symbols are explained in
[README.md](README.md#ladder-diagram-symbols).

```
 1  ||--[AI >= 153]--[AI < 240]---------->>

                                     O1
 2  >>--[TON 5s]--------------------(S)--||

                                     O1
 3  ||--[AI <= 145]--[TON 5s]-------(R)--||

 4  ||--[AI <= 119]--[AI >= 10]---------->>

                                     O2
 5  >>--[TON 5s]--------------------(S)--||

                                     O2
 6  ||--[AI >= 126]--[TON 5s]-------(R)--||

                                     O1
 7  ||--[AI >= 240]-----------------(R)--||

                                     O2
 8  ||--[AI < 10]-------------------(R)--||

 9  ||--[Indicator AI 80..-16.8 dp=1]----||

10  ||--[Indicator AI 0..255]------------||
```

## Code for Ladder::InitialLoad()

Made with `Tools/ladder.py cpp Demo/temperature-control.dat`.

<details>
<summary>Show code</summary>

```cpp
void Ladder::InitialLoad() {
    //  1  ||--[AI >= 153]--[AI < 240]-->>
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorGE(153, MapIO::AI));
        network->Append(new ComparatorLs(240, MapIO::AI));
        network->Append(new ContinuationIn());
        Append(network);
    }
    //                    O1
    //  2  >>--[TON 5s]--(S)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new TimerSecs(5));
        network->Append(new SetOutput(MapIO::O1));
        Append(network);
    }
    //                                 O1
    //  3  ||--[AI <= 145]--[TON 5s]--(R)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorLE(145, MapIO::AI));
        network->Append(new TimerSecs(5));
        network->Append(new ResetOutput(MapIO::O1));
        Append(network);
    }
    //  4  ||--[AI <= 119]--[AI >= 10]-->>
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorLE(119, MapIO::AI));
        network->Append(new ComparatorGE(10, MapIO::AI));
        network->Append(new ContinuationIn());
        Append(network);
    }
    //                    O2
    //  5  >>--[TON 5s]--(S)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ContinuationOut());
        network->Append(new TimerSecs(5));
        network->Append(new SetOutput(MapIO::O2));
        Append(network);
    }
    //                                 O2
    //  6  ||--[AI >= 126]--[TON 5s]--(R)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorGE(126, MapIO::AI));
        network->Append(new TimerSecs(5));
        network->Append(new ResetOutput(MapIO::O2));
        Append(network);
    }
    //                       O1
    //  7  ||--[AI >= 240]--(R)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorGE(240, MapIO::AI));
        network->Append(new ResetOutput(MapIO::O1));
        Append(network);
    }
    //                     O2
    //  8  ||--[AI < 10]--(R)--||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        network->Append(new ComparatorLs(10, MapIO::AI));
        network->Append(new ResetOutput(MapIO::O2));
        Append(network);
    }
    //  9  ||--[Indicator AI 80..-16.8 dp=1]----||
    {
        auto network = new Network(NetworkState::nsStopFromActive);
        auto indicator = new Indicator(MapIO::AI);
        indicator->SetLowScale(80);
        indicator->SetHighScale(-16.8f);
        indicator->SetDecimalPoint(1);
        indicator->UpdateScale();
        network->Append(indicator);
        Append(network);
    }
    // 10  ||--[Indicator AI 0..255]----||
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
}
```

</details>
