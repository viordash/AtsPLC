#pragma once

#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class LogicElementSerializer {
  protected:
  public:
    static size_t Serialize(const LogicElement *element, uint8_t *buffer, size_t buffer_size);
    static size_t Deserialize(const LogicElement *element, uint8_t *buffer, size_t buffer_size);
};
