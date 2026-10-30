# Third-Party Notices

The [PolyForm Noncommercial License](LICENSE) applies to the AtsPLC code only. The
components below are distributed under their own licenses, which are not changed by it.

## Bundled in the firmware source

| Component | Location | License |
|-----------|----------|---------|
| SSD1306 OLED driver (esp-open-rtos), (c) 2016 urx, Ruslan V. Uss, Zaltora | `PLC_esp8266/main/Display/ssd1306/` | MIT |
| LCD/OLED fonts library (esp-open-rtos), (c) 2016 Ruslan V. Uss, Zaltora | `PLC_esp8266/main/Display/fonts/fonts.*`, `fonts/tools/` | MIT |
| Terminus font, (c) Dimitar Toshkov Zhekov | `PLC_esp8266/main/Display/fonts/data/font_terminus_*` | SIL Open Font License 1.1 |
| BitOCRA font, (c) Aaron Christianson | `PLC_esp8266/main/Display/fonts/data/font_bitocra_*` | SIL Open Font License 1.1 |
| GLCD 5x7 font, (c) 2012 Majenko Technologies | `PLC_esp8266/main/Display/fonts/data/font_glcd_5x7.h` | BSD-style, see file header |
| Roboto font, (c) Google | `PLC_esp8266/main/Display/fonts/data/font_roboto_*` | Apache License 2.0 |

## Git submodules

| Component | Location | License |
|-----------|----------|---------|
| ESP8266_RTOS_SDK, (c) Espressif Systems | `SDK/ESP8266_RTOS_SDK/` | Apache License 2.0 |
| CppUTest, (c) Michael Feathers, James Grenning, Bas Vodde | `TestsFramework/CppUTest/` | BSD 3-Clause |
| MigrateAnyCppData, (c) Vitalii Orazov | `SDK/MigrateAnyCppData/` | MIT |

## Web interface

The Angular SPA in `Web/AtsPLC/` depends on npm packages (Angular, RxJS and others) listed in
`Web/AtsPLC/package.json`, each under its own license. The production build embedded into
the firmware contains their compiled code.
