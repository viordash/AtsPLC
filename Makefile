ROOT_DIR := $(shell git rev-parse --show-toplevel)

.PHONY: web app flash tests

WEB_INDEX := $(ROOT_DIR)/Web/output/browser/index.html

web:
	cd Web/AtsPLC && npm install && npm run build

$(WEB_INDEX):
	$(MAKE) web

app: $(WEB_INDEX)
	$(MAKE) -C PLC_esp8266 app

flash: $(WEB_INDEX)
	$(MAKE) -C PLC_esp8266 flash

tests: $(WEB_INDEX)
	$(MAKE) -C Tests_esp8266


export CF_SRC := PLC_esp8266/main Tests_esp8266/src
export CF_EXCLUDE_DIRS := PLC_esp8266/main/MigrateAnyData PLC_esp8266/main/Display/fonts Tests_esp8266/src/esp8266 Tests_esp8266/src/freertos
include $(ROOT_DIR)/check_format.mk