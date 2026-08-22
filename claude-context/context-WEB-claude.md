# Web-интерфейс

Браузер показывает копию экрана устройства на canvas и шлёт обратно нажатия кнопок.
Отдельного UI под браузер нет: это тот же ladder-редактор, что и на OLED.

## Сборка и встраивание

```
Web/AtsPLC/                 Angular-проект
Web/output/browser/         результат ng build (в git не попадает)
build-tools/mk/web_files.mk собирает список файлов и -D дефайны
PLC_esp8266/main/component.mk  COMPONENT_EMBED_FILES := $(WEB_DIST_SOURCES)
```

`ng build` кладёт результат в `Web/output/browser/`: `index.html`, `favicon.ico`,
`main-<hash>.js`, `styles-<hash>.css`. Имена js и css содержат хеш и меняются от сборки
к сборке, поэтому `web_files.mk` находит их через `wildcard` и передаёт в компилятор:

```
-Dmain_js_symbol=main_J6UY5DQJ_js   имя символа линкера
-Dmain_js_name="main-J6UY5DQJ.js"   имя файла в URI
```

`MainController` разворачивает их макросом `EMBEDDED_FILE_SYMBOL` в
`_binary_<symbol>_start` / `_binary_<symbol>_end`.

Тестовая сборка компилирует `MainController.cpp`, но символов встроенных файлов у неё нет -
в `Tests_esp8266/src/mocks.c` лежат заглушки, объявленные теми же макросами, поэтому они
не ломаются при смене хеша.

## Контроллеры

| Файл | URI | Метод | Назначение |
|------|-----|-------|------------|
| `MainController` | `/`, `/main`, `/index.html`, `/favicon.ico`, `/main-*.js`, `/styles-*.css` | GET | отдача SPA чанками по `SCRATCH_BUFSIZE` |
| `DisplayController` | `/devconfig` | GET | геометрия экрана и лимит одновременных запросов |
| `DisplayController` | `/bitmap` | GET | кадр framebuffer |
| `InputController` | `/keypress` | POST | `{"key":38,"down":1}` -> событие кнопки |
| `ProgramController` | `/program/upload` | POST | загрузка программы |
| `ProgramController` | `/program/download` | GET | выгрузка программы |
| `WorkModeController` | `/workmode` | GET / POST | текущий режим и его смена |
| `UpdateController` | `/update` | POST | OTA-обновление прошивки |

Ошибки отдаются как `{"error":"..."}` с типом `application/json` - `BaseController::SendError`.

## Передача кадра

`GET /bitmap` отдаёт сырой framebuffer, `DISPLAY_HEIGHT_IN_BYTES * DISPLAY_WIDTH` байт,
`application/octet-stream`. Заголовки:

- `ETag` - время последнего изменения кадра в миллисекундах. Если клиент прислал совпадающий
  `If-None-Match`, отдаётся `304` без тела.
- `X-DataPaging` - `{"offset":<индекс верхней сети>,"count":<всего сетей>}`, чтобы браузер
  мог нарисовать своё положение в программе.
- `X-ForceRefresh` - взводится, когда сменился режим работы. Клиент по нему перечитывает
  `/workmode`.

Пока идёт чтение кадра, задача отрисовки блокируется: `BeginRenderOnExternal` берёт
`render_mutex`, `EndRenderOnExternal` отпускает.

## Жизненный цикл сервера

`start_http_server()` вызывается из `WiFiService_Station` при подключении к станции,
`stop_http_server()` - при отключении. `RenderingService` берётся из
`Controller::GetRenderingService()`; если его ещё нет, сервер поднимается без
`DisplayController`, но с `UpdateController` - чтобы OTA-обновление оставалось доступным.
