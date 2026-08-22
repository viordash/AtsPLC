ROOT_DIR := $(shell git rev-parse --show-toplevel)

.PHONY: web app flash tests

# Собирает Web SPA один раз - прошивка встраивает её результат
web:
	cd Web/AtsPLC && npm install && npm run build

app:
	$(MAKE) -C PLC_esp8266 app

flash:
	$(MAKE) -C PLC_esp8266 flash

tests:
	$(MAKE) -C Tests_esp8266
