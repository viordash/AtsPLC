#include "LogicProgram/Inputs/ComparatorEq.h"

const Bitmap ComparatorEq::bitmap_passive = { //
    { 16,                                     // width
      16 },                                   // height
    { 0xDB, 0x00, 0x00, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x0A, 0x0A, 0x0A,
      0x0A, 0x00, 0x00, 0xDB, 0x80, 0xB6, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB6, 0x01 }
};
