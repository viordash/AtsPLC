#include "LogicProgram/MapIO.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// const char *MapIONames[] = { "DI", "AI", "O1", "O2", "V1", "V2", "V3", "V4" };


std::map<MapIO, const char *> MapIONames = { { MapIO::DI, "DI" }, { MapIO::AI, "AI" },
                                             { MapIO::O1, "O1" }, { MapIO::O2, "O2" },
                                             { MapIO::V1, "V1" }, { MapIO::V2, "V2" },
                                             { MapIO::V3, "V3" }, { MapIO::V4, "V4" } };