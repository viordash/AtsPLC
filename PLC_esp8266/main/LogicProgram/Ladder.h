#pragma once

#include "LogicProgram/Network.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

class Ladder : std::vector<Network *> {
  protected:

  public:
    Ladder();
    ~Ladder();
};
