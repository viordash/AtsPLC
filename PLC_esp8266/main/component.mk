#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_PRIV_INCLUDEDIRS := . params Display
COMPONENT_SRCDIRS := . MigrateAnyData HttpServer Display Display/ssd1306 Display/fonts

FONTS_GLCD_5X7 := 1
FONT_FACE_TERMINUS_6X12_ISO8859_1 := 1