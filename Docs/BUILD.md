# Build, tooling, VSCode

## Build layout

```
Makefile                        root entry point
PLC_esp8266/Makefile            firmware, ESP8266_RTOS_SDK
Tests_esp8266/Makefile          unit tests, CppUTest
Web/AtsPLC/                     Angular SPA
build-tools/mk/web_files.mk     list of embedded SPA files and -D defines
```

Everything that builds the project but isn't the project itself lives in `build-tools/`.
A new `.mk` file goes there too and is included as
`$(ROOT_DIR)/build-tools/mk/<name>.mk`, where `ROOT_DIR` is resolved via
`git rev-parse --show-toplevel`.

## One-time setup

```bash
cd SDK
tar -xzf xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz
```

SDK patches are applied automatically on the first firmware build.

## Root Makefile

```bash
make web      # npm install && ng build, output in Web/output/browser
make app      # firmware
make flash    # firmware + flash over USB
make tests    # unit tests
```

`make web` is only needed once after frontend changes: the firmware embeds the already
built output.

## Firmware

```bash
cd PLC_esp8266
make -j$(nproc) app          # build
make -j$(nproc) flash        # flash over USB
make -j$(nproc) size         # flash usage breakdown
```

OTA update - `POST /update` with the firmware file, see
[WEB_INTERFACE.md](WEB_INTERFACE.md).

## Tests

```bash
cd Tests_esp8266
make -j$(nproc)
./output/main                        # whole suite
./output/main -sg <GroupName>        # one group
./output/main -sn <TestName>         # one test
```

A new firmware file that needs to be covered by tests is added via `#include` in
`Tests_esp8266/src/LogicProgram.cpp`. `Tests_esp8266/Makefile` includes
`PLC_esp8266/main/component.mk`, so build defines reach the tests too.

Tests build `HttpServer/MainController.cpp`, which needs the embedded SPA symbols.
The firmware creates them via `COMPONENT_EMBED_FILES`; tests use a `build/web/%.o` rule
(`ld -r -b binary`). The root `make app` / `make flash` / `make tests` build the SPA
themselves if it's missing; calling `PLC_esp8266` or `Tests_esp8266` directly stops with
a hint to run `make web` first.

## Size

| What | Value |
|------|-------|
| Firmware partition | 1 MB (`ota_0` / `ota_1`) |
| Firmware with embedded SPA | about 855 KiB (`build/AtsPLC-esp8266.bin`) |

## VSCode

Build and debug tasks live in `.vscode/`. Firmware debugging goes through `esp_gdbstub`.
