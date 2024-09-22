#pragma once

#include "Display/Common.h"
#include "Display/EditableElement.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include "LogicProgram/Serializer/TypeValueElement.h"
#include <mutex>
#include <stdint.h>
#include <unistd.h>

class Network;

class LogicElement: public EditableElement {
  protected:
    std::recursive_mutex lock_mutex;
    LogicItemState state;

    friend Network;

  public:
    static const uint8_t MinValue = 0;
    static const uint8_t MaxValue = 250;

    LogicElement();
    virtual ~LogicElement();

    virtual bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) = 0;
    virtual bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) = 0;

    virtual size_t Serialize(uint8_t *buffer, size_t buffer_size) = 0;
    virtual size_t Deserialize(uint8_t *buffer, size_t buffer_size) = 0;

    virtual TvElementType GetElementType() = 0;
};
