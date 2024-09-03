#pragma once

#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

class Network : public LogicElement, public std::vector<LogicElement *> {
  private:
    uint8_t network_number;
    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override final;

  public:
    const static size_t MinElementsCount = 2;
    const static size_t MaxElementsCount = 5;
    Network();
    virtual ~Network();

    void SetNumber(uint8_t network_number);
    void ChangeState(LogicItemState state);

    bool DoAction();
    bool Render(uint8_t *fb);

    void Append(LogicElement *element);

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;
};
