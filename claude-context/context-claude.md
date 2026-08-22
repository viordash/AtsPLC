# Project Context

AtsPLC — программируемый логический контроллер на ESP8266. Программа в виде Ladder Diagram
редактируется прямо на устройстве кнопками, на OLED-экране 128x64. Есть web-интерфейс:
браузер копирует framebuffer устройства на canvas и шлёт обратно нажатия кнопок.

AtsPLC — прародитель [SuPLC](https://github.com/viordash/SuPLC). SuPLC развивается как
многоплатформенный (ESP32 / ESP8266 / Linux) и несёт максимальный набор функционала;
AtsPLC остаётся одноплатформенным с базовым набором элементов.

## Architecture Overview

```
+-----------------------------------------------------------+
|                    PLC_esp8266/main                        |
|  LogicProgram (Ladder -> Network -> LogicElement)          |
|  Display (framebuffer, ssd1306, шрифты, битмапы)           |
|  HttpServer (REST для web) - WiFi - Datetime               |
|  Maintenance (сервисный режим) - hotreload - settings      |
|  sys_gpio / storage / redundant_storage                    |
+-----------------------------------------------------------+
                 ^                          ^
                 | ESP8266_RTOS_SDK         | Web/AtsPLC (Angular SPA)
                 |                          |
        +----------------+        встраивается в прошивку
        | esp8266 (4 MB) |        через COMPONENT_EMBED_FILES
        +----------------+
```

## Project Structure

```
AtsPLC/
+-- PLC_esp8266/
|   +-- main/               # весь код прошивки
|   |   +-- LogicProgram/   # ядро PLC: Ladder / Network / LogicElement
|   |   +-- Display/        # графика, шрифты, ssd1306, ListBox, ScrollBar
|   |   +-- HttpServer/     # REST API и отдача SPA
|   |   +-- WiFi/           # WiFiService: станция, сканер, точка доступа
|   |   +-- Datetime/       # служба даты и времени, SNTP
|   |   +-- Maintenance/    # сервисный режим, backup/restore/reset
|   |   +-- DataMigrations/ # миграции структуры настроек
|   |   +-- params/         # разметка разделов flash
|   +-- partitions.csv
|   +-- Makefile
+-- Tests_esp8266/          # юнит-тесты на CppUTest
+-- Web/AtsPLC/             # Angular SPA
+-- SDK/                    # ESP8266_RTOS_SDK, toolchain, MigrateAnyCppData
+-- build-tools/mk/         # включаемые фрагменты Makefile
+-- Docs/                   # документация и схемы
+-- Makefile                # корневая точка входа: web / app / flash / tests
+-- claude-context/         # контекстные файлы для Claude Code
```

## Key Concepts

| Concept | Description |
|---------|-------------|
| Ladder / Network / LogicElement | Дерево программы: `Ladder` содержит `Network[]`, каждая `Network` — цепочку `LogicElement` (входы -> выходы) |
| LogicElementFactory / Serializer | Бинарная сериализация программы по `TvElementType` (`LogicProgram/Serializer/`) |
| WorkMode | Stop / Run / Debug. Хранится в файле программы (`Stop`/`Run`) и в RTC (`enable_debug`) |
| NetworkState / LogicItemState | Семя цепочки даёт `Network`; `lisStop` замораживает исполнение элементов |
| ServiceModeHandler | Сервисный режим: Work mode, SmartConfig, Backup/Restore (4 слота), Reset |
| hotreload_service | Состояние UI и текущее время переживают перезапуск через RTC RAM |
| CurrentSettings | Единая структура настроек устройства (`settings.h`), сохраняется через `redundant_storage` |

## Ограничения

| Что | Значение |
|-----|----------|
| Сетей в программе | 1..80 |
| Элементов в сети | 1..5 |
| Видимых сетей на экране | 2 |
| Переменных | V1..V4 |
| Размер прошивки | 1 МБ (`ota_0` / `ota_1`) |
| Размер загружаемой программы | 4 КБ (`PROGRAM_MAXSIZE`) |

## Detailed Context Files

| File | When to read |
|------|--------------|
| [context-ESP8266-claude.md](context-ESP8266-claude.md) | Сборка и отладка прошивки, дисплей, GPIO, разделы flash |
| [context-WEB-claude.md](context-WEB-claude.md) | REST API, Angular SPA, встраивание фронтенда в прошивку |

## Documentation

Общий обзор — [README.md](../README.md). Подробности по темам — `Docs/`:
[BUILD.md](../Docs/BUILD.md), [LADDER_ELEMENTS.md](../Docs/LADDER_ELEMENTS.md),
[WORK_MODES.md](../Docs/WORK_MODES.md), [SERVICE_MODE.md](../Docs/SERVICE_MODE.md),
[WIFI.md](../Docs/WIFI.md), [WEB_INTERFACE.md](../Docs/WEB_INTERFACE.md).

## Связь с SuPLC

Перенос изменений из SuPLC ведётся по списку, который лежит в самом SuPLC:
`claude-context/atsplc-sync-backlog.md`. Там же зафиксировано, что переносить не нужно
(логирование, отсутствующие в AtsPLC элементы `Controls/`, многоплатформенный BSP-слой),
и по каждому перенесённому пункту — чем реализация в AtsPLC отличается от исходной.
