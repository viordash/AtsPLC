#pragma once

#include <map>

enum MapIO { DI = 0, AI, O1, O2, V1, V2, V3, V4 };
// extern const char *MapIONames[];

extern std::map<MapIO, const char *> MapIONames;
