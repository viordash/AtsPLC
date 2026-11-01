define sanitize_symbol
$(subst -,_,$(subst .,_,$(1)))
endef

WEB_DIST := $(ROOT_DIR)/Web/output/browser

FAVICON_ICO_FILE = $(wildcard $(WEB_DIST)/favicon.ico)
INDEX_HTML_FILE = $(wildcard $(WEB_DIST)/index.html)
MAIN_JS_FILE = $(wildcard $(WEB_DIST)/main-*.js)
STYLES_CSS_FILE = $(wildcard $(WEB_DIST)/styles-*.css)

WEB_DIST_SOURCES := $(FAVICON_ICO_FILE)
WEB_DIST_SOURCES += $(INDEX_HTML_FILE)
WEB_DIST_SOURCES += $(MAIN_JS_FILE)
WEB_DIST_SOURCES += $(STYLES_CSS_FILE)

ifdef BUILD
WEB_DIST_OBJECTS := $(patsubst $(WEB_DIST)/%,$(BUILD)/web/%.o,$(WEB_DIST_SOURCES))
endif

WEB_NOT_REQUIRED_GOALS := clean cleanlib project_prepare
ifeq ($(strip $(WEB_DIST_SOURCES)),)
ifeq ($(filter $(WEB_NOT_REQUIRED_GOALS),$(MAKECMDGOALS)),)
$(error Web SPA is not built ($(WEB_DIST) is empty): run `make web` from the repo root)
endif
endif

FAVICON_ICO_SYMBOL:=-Dfavicon_ico_symbol=$(call sanitize_symbol,$(notdir $(FAVICON_ICO_FILE)))
FAVICON_ICO_NAME:=-Dfavicon_ico_name=\"$(notdir $(FAVICON_ICO_FILE))\"

INDEX_HTML_SYMBOL:=-Dindex_html_symbol=$(call sanitize_symbol,$(notdir $(INDEX_HTML_FILE)))
INDEX_HTML_NAME:=-Dindex_html_name=\"$(notdir $(INDEX_HTML_FILE))\"

MAIN_JS_SYMBOL:=-Dmain_js_symbol=$(call sanitize_symbol,$(notdir $(MAIN_JS_FILE)))
MAIN_JS_NAME:=-Dmain_js_name=\"$(notdir $(MAIN_JS_FILE))\"

STYLES_CSS_SYMBOL:=-Dstyles_css_symbol=$(call sanitize_symbol,$(notdir $(STYLES_CSS_FILE)))
STYLES_CSS_NAME:=-Dstyles_css_name=\"$(notdir $(STYLES_CSS_FILE))\"

COMMON_FLAGS += $(FAVICON_ICO_SYMBOL) $(FAVICON_ICO_NAME)
COMMON_FLAGS += $(INDEX_HTML_SYMBOL) $(INDEX_HTML_NAME)
COMMON_FLAGS += $(MAIN_JS_SYMBOL) $(MAIN_JS_NAME)
COMMON_FLAGS += $(STYLES_CSS_SYMBOL) $(STYLES_CSS_NAME)
