#pragma once

#include "LogicProgram/Network.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

class NetworksList : std::vector<Network *> {
  protected:

  public:
    NetworksList();
    ~NetworksList();
};
