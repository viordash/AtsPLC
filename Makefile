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
