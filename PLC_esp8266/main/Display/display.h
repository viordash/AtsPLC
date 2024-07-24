#pragma once

#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

void display_init();

void display_demo_0();
void display_demo_1();
void display_demo_2();
void display_demo(int8_t x, int8_t y);

#ifdef __cplusplus
}
#endif