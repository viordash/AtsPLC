#pragma once

#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class LogicElementFactory {
  protected:
  public:
    static LogicElement *Create(uint8_t *buffer, size_t buffer_size);
};
