#include "LogicProgram/Inputs/ComparatorNe.h"

const Bitmap ComparatorNe::bitmap_active = { //
    { 16,                                    // width
      16 },                                  // height
    { 0xFF, 0x00, 0x00, 0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x11, 0x0A,
      0x04, 0x00, 0x00, 0xFF, 0x80, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01 }
};
