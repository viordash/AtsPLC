# Demo programs

Ready-to-use ladder programs. Each program has a `.dat` file and a `.md` file with the same
name. The `.dat` file is the program you upload to the device. The `.md` file describes the
wiring and the logic, and shows the program as text and as C++ code.

| Program | About |
|---------|-------|
| [aquarium](aquarium.md) | light and filter pump schedule with a photo sensor |
| [security-alarm](security-alarm.md) | door alarm with a siren and a state LED, the owner's phone works as the key |

## How to load a program

Switch the controller to Stop in the web interface and upload the `.dat` file, see
[WEB_INTERFACE.md](../Docs/WEB_INTERFACE.md#program). Switch to Run to save it to flash.

## Ladder diagram symbols

Each `.md` file shows the program as a ladder diagram drawn with text characters. An
address above a symbol is the address of that element.

| Symbol | Element |
|--------|---------|
| `\|\|` | power rail |
| `] [` | normally open contact (InputNO) |
| `]/[` | normally closed contact (InputNC) |
| `[V2 >= 13]` | comparator |
| `[TON 60s]` | timer (TimerSecs / TimerMSecs) |
| `[SQW off 500ms on 500ms]` | SquareWaveGenerator: pause, then pulse |
| `( )` | DirectOutput |
| `(S)` / `(R)` | SetOutput / ResetOutput |
| `(+)` / `(-)` | IncOutput / DecOutput |
| `>>` at the end / at the start | ContinuationIn / ContinuationOut |
| `[Indicator ...]`, `[DateTime ...]`, `[WiFiAp ...]`, `[Settings ...]` | other elements |

## How to add a program

1. Make the program on the device, download it from the web interface and put it here as
   `<name>.dat`.
2. Write `<name>.md`. Use `Tools/ladder.py` to make the diagram and the code:

```bash
Tools/ladder.py ascii Demo/<name>.dat   # ladder diagram
Tools/ladder.py cpp Demo/<name>.dat     # code in the style of Ladder::InitialLoad()
```
