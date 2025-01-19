#pragma once

#include "LogicProgram/Inputs/CommonInput.h"
#include <stdint.h>
#include <unistd.h>

class WiFiStation : public LogicElement {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;
    const static Bitmap bitmap_error;

  public:
    typedef enum { //
        ciepi_None = EditableElement::EditingPropertyId::cepi_None,
    } EditingPropertyId;

    const uint8_t LeftPadding = 4;
    explicit WiFiStation();
    ~WiFiStation();

    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override final;
    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    static WiFiStation *TryToCast(LogicElement *logic_element);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
};
