#pragma once

#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

class InputElement;
class CommonInput;
class CommonTimer;
class CommonComparator;
class CommonOutput;
class Indicator;
class WiFiBinding;
class WiFiApBinding;

class ElementsBox : public LogicElement, public std::vector<LogicElement *> {
  protected:
    uint8_t place_width;
    uint8_t source_element_width;
    int selected_index;
    bool force_do_action_result;

    void DetachElement(LogicElement *element);
    void CalcEntirePlaceWidth(LogicElement *source_element);
    void Fill(LogicElement *source_element, bool hide_output_elements);
    void AppendStandartElement(LogicElement *source_element,
                               TvElementType element_type,
                               uint8_t *frame_buffer);
    void CopyParamsToInputElement(LogicElement *source_element, InputElement *input);
    bool CopyParamsToCommonComparator(LogicElement *source_element,
                                      CommonComparator *common_comparator);
    bool CopyParamsToCommonTimer(LogicElement *source_element, CommonTimer *common_timer);
    bool CopyParamsToIndicator(LogicElement *source_element, Indicator *indicator);
    bool CopyParamsToWiFiBinding(LogicElement *source_element, WiFiBinding *binding);
    bool CopyParamsToWiFiApBinding(LogicElement *source_element, WiFiApBinding *binding);
    void TakeParamsFromStoredElement(LogicElement *source_element, LogicElement *new_element);

  public:
    ElementsBox(uint8_t fill_wire, LogicElement *source_element, bool hide_output_elements);
    virtual ~ElementsBox();

    LogicElement *GetSelectedElement();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override final;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    void SelectPrior() override final;
    void SelectNext() override final;
    void PageUp() override final;
    void PageDown() override final;
    void Change() override final;
    void Option() override final;
    bool EditingCompleted();
};
