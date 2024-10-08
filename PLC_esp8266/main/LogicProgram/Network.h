#pragma once

#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

class Network : public std::vector<LogicElement *> {
  protected:
    LogicItemState state;

  public:
    const static size_t MinElementsCount = 2;
    const static size_t MaxElementsCount = 5;
    explicit Network();
    Network(LogicItemState state);
    virtual ~Network();

    void ChangeState(LogicItemState state);

    virtual bool DoAction();
    virtual bool Render(uint8_t *fb, uint8_t network_number);

    void Append(LogicElement *element);

    size_t Serialize(uint8_t *buffer, size_t buffer_size);
    size_t Deserialize(uint8_t *buffer, size_t buffer_size);
};
