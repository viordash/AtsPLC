#include "LogicProgram/ComparatorGE.h"
#include "Display/bitmaps/cmp_greate_or_equal_active.h"
#include "Display/bitmaps/cmp_greate_or_equal_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorGE::ComparatorGE(const MapIO io_adr, const Point &incoming_point)
    : InputBase(io_adr, incoming_point) {
}
ComparatorGE::ComparatorGE(const MapIO io_adr, InputBase &prior_item)
    : InputBase(io_adr, prior_item) {
}

ComparatorGE::~ComparatorGE() {
}

bool ComparatorGE::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *ComparatorGE::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &ComparatorGE::bitmap_active;

        default:
            return &ComparatorGE::bitmap_passive;
    }
}

void ComparatorGE::Render(uint8_t *fb) {
    InputBase::Render(fb);

    uint8_t x_pos = incoming_point.x + LeftPadding + LabeledLogicItem::width + 2;
    // draw_text_f4X7(x_pos, incoming_point.y + 1, "429");
    draw_text_f5X7(x_pos, incoming_point.y + 2, "42");
}