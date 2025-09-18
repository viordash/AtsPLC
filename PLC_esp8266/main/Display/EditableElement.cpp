#include "Display/EditableElement.h"
#include "Display/bitmaps/copy_cursor_0.h"
#include "Display/bitmaps/copy_cursor_1.h"
#include "Display/bitmaps/delete_cursor_0.h"
#include "Display/bitmaps/delete_cursor_1.h"
#include "Display/bitmaps/disable_cursor_0.h"
#include "Display/bitmaps/disable_cursor_1.h"
#include "Display/bitmaps/element_cursor_0.h"
#include "Display/bitmaps/element_cursor_1.h"
#include "Display/bitmaps/element_cursor_2.h"
#include "Display/bitmaps/moving_up_down_0.h"
#include "Display/bitmaps/moving_up_down_1.h"
#include "Display/display.h"
#include "EditableElement.h"
#include "LogicProgram/Controller.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

EditableElement::EditableElement() {
    editable_state = EditableElement::ElementState::des_Regular;
    editing_property_id = EditingPropertyId::cepi_None;
    blink = false;
}

EditableElement::~EditableElement() {
}

void EditableElement::Select() {
    editable_state = EditableElement::ElementState::des_Selected;
}
void EditableElement::CancelSelection() {
    editable_state = EditableElement::ElementState::des_Regular;
}

void EditableElement::BeginEditing() {
    editable_state = EditableElement::ElementState::des_Editing;
    editing_property_id = EditingPropertyId::cepi_None;
}
void EditableElement::EndEditing() {
    editable_state = EditableElement::ElementState::des_Regular;
}

bool EditableElement::Selected() {
    return editable_state == EditableElement::ElementState::des_Selected;
}

void EditableElement::Render(FrameBuffer *fb, Point *start_point) {
    const Bitmap *bitmap = GetCursorBitmap(fb);
    if (bitmap != NULL) {
        draw_bitmap(fb, start_point->x + 1, start_point->y + 2, bitmap);
    }
}

const Bitmap *EditableElement::GetCursorBitmap(FrameBuffer *fb) {
    switch (editable_state) {
        case EditableElement::ElementState::des_Selected:
            return Blinking_50(fb) ? &bitmap_selecting_blink_0 : &bitmap_selecting_blink_1;

        case EditableElement::ElementState::des_Editing:
            return &bitmap_selecting_blink_2;

        case EditableElement::ElementState::des_AdvancedSelectMove:
            return Blinking_50(fb) ? &bitmap_moving_up_down_0 : &bitmap_moving_up_down_1;

        case EditableElement::ElementState::des_AdvancedSelectCopy:
            return Blinking_50(fb) ? &bitmap_copy_cursor_0 : &bitmap_copy_cursor_0;

        case EditableElement::ElementState::des_AdvancedSelectDelete:
            return Blinking_50(fb) ? &bitmap_delete_cursor_0 : &bitmap_delete_cursor_1;

        case EditableElement::ElementState::des_AdvancedSelectDisable:
            return Blinking_50(fb) ? &bitmap_disable_cursor_0 : &bitmap_disable_cursor_1;

        case EditableElement::ElementState::des_Moving:
            return &bitmap_moving_up_down_0;

        case EditableElement::ElementState::des_Copying:
            return &bitmap_copy_cursor_0;

        case EditableElement::ElementState::des_Deleting:
            return &bitmap_delete_cursor_0;

        case EditableElement::ElementState::des_Disabling:
            return &bitmap_disable_cursor_0;

        default:
            return NULL;
    }
}

uint16_t EditableElement::GetCursorWidth(FrameBuffer *fb) {
    const Bitmap *bitmap = GetCursorBitmap(fb);
    if (bitmap != NULL) {
        return bitmap->size.width;
    }
    return 0;
}

EditableElement::ElementState EditableElement::GetEditable_state() {
    return editable_state;
}

bool EditableElement::Editing() {
    return editable_state == EditableElement::ElementState::des_Editing;
}

bool EditableElement::InEditingProperty() {
    return editing_property_id != EditableElement::EditingPropertyId::cepi_None;
}

bool EditableElement::Blinking_50(FrameBuffer *fb) {
    const int blink_timer_ms = 400;

    const int offset_Blinking_50 = 50;
    auto blinking = Controller::RequestWakeupMs((void *)(this + offset_Blinking_50),
                                                blink_timer_ms,
                                                ProcessWakeupRequestPriority::pwrp_Idle);
    if (blinking) {
        Controller::WakeupProcessTask();
        blink = !blink;
        fb->has_changes = true;
    }
    return blink;
}
