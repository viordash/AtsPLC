#pragma once

#include "LogicProgram/Inputs/CommonInput.h"
#include "LogicProgram/Inputs/CommonTimer.h"
#include "LogicProgram/LogicElement.h"
#include "LogicProgram/Outputs/CommonOutput.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

class ElementsBox : public LogicElement, public std::vector<LogicElement *> {
  protected:
    uint8_t place_width;
    LogicElement *stored_element;
    int selected_index;
    bool force_do_action_result;

    uint8_t CalcEntirePlaceWidth(uint8_t fill_wire, LogicElement *stored_element);
    void Fill();
    void AppendStandartElement(TvElementType element_type, uint8_t *frame_buffer);
    bool MatchedToStoredElement(TvElementType element_type);
    bool CopyParamsToCommonInput(CommonInput *common_input);
    bool CopyParamsToCommonTimer(CommonTimer *common_timer);
    bool CopyParamsToCommonOutput(CommonOutput *common_output);
    void TakeParamsFromStoredElement(LogicElement *new_element);

  public:
    ElementsBox(uint8_t fill_wire, LogicElement *stored_element);
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
    bool EditingCompleted();
};
