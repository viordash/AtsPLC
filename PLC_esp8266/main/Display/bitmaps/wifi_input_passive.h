#include "LogicProgram/Inputs/WiFiInput.h"

const Bitmap WiFiInput::bitmap_passive = { //
    { 8,                                   // width
      16 },                                // height
    { 0xDB,
      0x00,
      0x00,
      0x00,
      0x40,
      0x10,
      0x04,
      0xDB,
      0xB6,
      0x10,
      0x04,
      0x01,
      0x00,
      0x00,
      0x00,
      0xB6 }
};
