# Demo programs

Ready-to-use ladder programs. Each program has a `.dat` file and a `.md` file with the same
name. The `.dat` file is the program you upload to the device. The `.md` file describes the
wiring and the logic, and shows the program as text and as C++ code.

| Program | About |
|---------|-------|
| [aquarium](aquarium.md) | light and filter pump schedule with a photo sensor |

## How to load a program

Switch the controller to Stop in the web interface and upload the `.dat` file, see
[WEB_INTERFACE.md](../Docs/WEB_INTERFACE.md#program). Switch to Run to save it to flash.

## How to add a program

1. Make the program on the device and download it from the web interface.
2. Put the file here as `<name>.dat`.
3. Write `<name>.md`. Use `Tools/ladder.py` to make the program listing and the code:

```bash
Tools/ladder.py dump Demo/<name>.dat   # one network per line
Tools/ladder.py cpp Demo/<name>.dat    # code in the style of Ladder::InitialLoad()
```
