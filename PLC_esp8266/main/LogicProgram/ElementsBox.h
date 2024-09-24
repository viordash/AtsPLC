#pragma once

#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

class ElementsBox : public std::vector<LogicElement *> {
  protected:
    uint8_t place_width;
    LogicElement *stored_element;

    void Fill();
    void AppendStandartElement(TvElementType element_type, uint8_t *frame_buffer);
    bool MatchedToStoredElement(TvElementType element_type);
    void TakeParamsFromStoredElement(LogicElement * new_element);

  public:
    ElementsBox(uint8_t place_width, LogicElement *stored_element);
    virtual ~ElementsBox();
};
