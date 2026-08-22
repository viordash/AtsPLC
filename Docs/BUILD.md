# Сборка, тулинг, VSCode

## Структура сборки

```
Makefile                        корневая точка входа
PLC_esp8266/Makefile            прошивка, ESP8266_RTOS_SDK
Tests_esp8266/Makefile          юнит-тесты, CppUTest
Web/AtsPLC/                     Angular SPA
build-tools/mk/web_files.mk     список встраиваемых файлов SPA и -D дефайны
```

Всё, что собирает проект, но не является самим проектом, лежит в `build-tools/`.
Новый `.mk` кладётся туда же и подключается как
`$(ROOT_DIR)/build-tools/mk/<name>.mk`, где `ROOT_DIR` вычисляется через
`git rev-parse --show-toplevel`.

## Первоначальная подготовка (один раз)

```bash
cd SDK
tar -xzf xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz
```

Патчи SDK применяются автоматически при первой сборке прошивки.

## Корневой Makefile

```bash
make web      # npm install && ng build, результат в Web/output/browser
make app      # прошивка
make flash    # прошивка + заливка через USB
make tests    # юнит-тесты
```

`make web` нужен один раз после правок фронтенда: прошивка встраивает уже собранный результат.

## Прошивка

```bash
cd PLC_esp8266
make -j$(nproc) app          # сборка
make -j$(nproc) flash        # заливка через USB
make -j$(nproc) size         # разбор занятого места
```

После правки любого заголовка перед сборкой нужно `rm -rf PLC_esp8266/build/main`:
зависимости от `.h` сборка не отслеживает.

OTA-обновление - `POST /update` с файлом прошивки, см.
[WEB_INTERFACE.md](WEB_INTERFACE.md).

## Тесты

```bash
cd Tests_esp8266
make -j$(nproc)
./output/main                        # весь набор
./output/main -sg <GroupName>        # одна группа
./output/main -sn <TestName>         # один тест
```

Новый файл прошивки, попадающий в тесты, добавляется через `#include` в
`Tests_esp8266/src/LogicProgram.cpp`. `Tests_esp8266/Makefile` подключает
`PLC_esp8266/main/component.mk`, поэтому дефайны сборки доезжают до тестов.

## Размер

| Что | Значение |
|-----|----------|
| Раздел под прошивку | 1 МБ (`ota_0` / `ota_1`) |
| Прошивка с встроенной SPA | около 880 КБ |

## VSCode

Задачи сборки и отладки лежат в `.vscode/`. Отладка прошивки - через `esp_gdbstub`.
