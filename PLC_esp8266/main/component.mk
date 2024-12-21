#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_PRIV_INCLUDEDIRS := . params Display LogicProgram WiFi
COMPONENT_SRCDIRS := . MigrateAnyData HttpServer Display Display/ssd1306 Display/fonts Display/bitmaps \
	LogicProgram LogicProgram/Inputs LogicProgram/Outputs LogicProgram/Serializer LogicProgram/Bindings \
	WiFi
