#pragma once

#include <stdint.h>
#include <unistd.h>

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

#ifdef __cplusplus
extern "C" {
#endif

void display_init();

void display_demo_0();
void display_demo_1();
void display_demo_2();
void display_demo(int8_t x, int8_t y);

void draw_demo(int8_t x, int8_t y, const uint8_t *xbm_data, int8_t xbm_width, int8_t xbm_height);
uint8_t *get_display_buffer();
void begin_render();
void end_render();
void draw_text_f5X7(uint8_t x, uint8_t y, const char *text);
void draw_text_f6X12(uint8_t x, uint8_t y, const char *text);
void draw_network(int8_t x, int8_t y, uint8_t w);

#ifdef __cplusplus
}
#endif