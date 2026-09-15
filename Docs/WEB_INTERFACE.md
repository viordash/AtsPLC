# Web interface

The browser shows a copy of the device screen and sends button presses back. There's no
separate browser UI: it's the same ladder editor as on the OLED, just bigger.

The server comes up once the device connects to a WiFi station, and goes down on
disconnect. The address is whatever the router assigned.

## Features

- live device screen;
- Up / Down / Select buttons from the keyboard;
- downloading and uploading the program as a file;
- switching the work mode;
- OTA firmware update.

## REST API

| URI | Method | Purpose |
|-----|--------|---------|
| `/` , `/main` | GET | SPA page |
| `/devconfig` | GET | screen geometry and the concurrent request limit |
| `/bitmap` | GET | screen frame |
| `/keypress` | POST | button press or release |
| `/program/download` | GET | download the program as a file |
| `/program/upload` | POST | upload a program |
| `/workmode` | GET | current work mode |
| `/workmode` | POST | change the work mode |
| `/update` | POST | OTA firmware update |

Errors come back as `{"error":"..."}` with type `application/json`.

## Screen

`GET /devconfig` returns what the browser needs to draw the screen:

```json
{"display":{"height":64,"width":128,"id":"ssd1306",
            "area_top":17,"area_width":126,"rails_height":25},
 "request_limit":2}
```

`GET /bitmap` returns the raw framebuffer, one bit per pixel, `application/octet-stream`.

The frame isn't sent needlessly: the response carries an `ETag` with the time of the last
change, and if the browser sent a matching `If-None-Match`, the server replies `304` with
no body.

The `X-DataPaging` header reports which part of the program is currently on screen:

```json
{"offset":0,"count":7}
```

The `X-ForceRefresh` header is present in every response (including `304`) and carries a
counter that increases on every work mode change. The browser compares it to the previous
value and, on a mismatch, re-reads `/workmode`.

## Buttons

```
POST /keypress
{"key":38,"down":1}
```

`key` - key code: 38 arrow up, 40 arrow down, 16 right Shift.
`down` - 1 press, 0 release. The event goes into the same event group as the physical
buttons, so handling logic is shared.

## Program

`GET /program/download` returns the program as a file named `program.dat`.

`POST /program/upload` accepts the same kind of file, up to `PROGRAM_MAXSIZE` (4 KB). The
uploaded program is parsed first; if parsing fails, `400` is returned and the device
re-reads the previous program.

Upload is only allowed in Stop mode: if the controller is running (Run or Debug), `400` is
returned with an explanation, and the request body isn't even read. In the web interface
the Upload button is disabled in that state. Upload is also forbidden while the program is
being edited on the device - same as changing the work mode.

## Work mode

```
GET  /workmode          -> {"mode":0}
POST /workmode          <- {"mode":2}
```

`0` Stop, `1` Run, `2` Debug. See [WORK_MODES.md](WORK_MODES.md) for mode details.

Changing the mode is forbidden while the program is being edited on the device: in that
case `400` is returned with an explanation. The switch itself happens in the controller
task, not the HTTP task: the controller receives an event and switches the mode in its own
cycle.

## OTA update

`POST /update` with the firmware file. The size is checked before writing starts: a file
larger than the partition (`FIRMWARE_MAXSIZE`, 1 MB) is rejected with `400`. After a
successful write the boot partition switches to the new one and the device reboots.

## Frontend

The Angular app lives in `Web/AtsPLC/`. It's built separately and embedded into the
firmware:

```bash
make web
make app
```

The built `index.html`, `favicon.ico`, `main-*.js`, `styles-*.css` end up in the image via
`COMPONENT_EMBED_FILES` and are served straight from flash, with no filesystem.
