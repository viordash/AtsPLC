#pragma once

#include "LogicProgram/Inputs/InputBase.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

class IncomeRail : public InputBase, public std::vector<LogicElement *> {
  private:
    uint8_t network_number;
    bool DoAction(bool prev_changed) override final;

  public:
    explicit IncomeRail(const Controller *controller, uint8_t network_number, LogicItemState state);
    virtual ~IncomeRail();

    Point OutcomingPoint() override final;
    bool DoAction();

    bool Render(uint8_t *fb) override final;

    void Append(LogicElement *element);

    // std::list<LogicElement *> elements;
};
