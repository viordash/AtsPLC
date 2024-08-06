#pragma once

#include <stdint.h>
#include <unistd.h>

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define INCOME_RAIL_TOP 16
#define INCOME_RAIL_HEIGHT 25
#define OUTCOME_RAIL_TOP 16
#define OUTCOME_RAIL_HEIGHT 25
#define PROGRESS_BAR_HEIGHT 10
#define PROGRESS_BAR_WIDTH 3

#ifdef __cplusplus
extern "C" {
#endif

#include "Display/Common.h"

void display_init();

void display_demo_0(bool active);
void display_demo_1();
void display_demo_2();
void display_demo(int8_t x, int8_t y);

void draw_demo(int8_t x, int8_t y, const uint8_t *xbm_data, uint8_t xbm_width, uint8_t xbm_height);
uint8_t *get_display_buffer();
void begin_render();
void end_render();
void draw_text_f4X7(uint8_t x, uint8_t y, const char *text);
void draw_text_f5X7(uint8_t x, uint8_t y, const char *text);
void draw_text_f6X12(uint8_t x, uint8_t y, const char *text);
void draw_active_network(int8_t x, int8_t y, uint8_t w);
void draw_passive_network(int8_t x, int8_t y, uint8_t w, bool inverse_dash);
void draw_income_rail(uint8_t network_number);
void draw_outcome_rail(uint8_t network_number);
void draw_progress_bar(int8_t x, int8_t y, uint8_t percent);
void draw_vert_line(int8_t x, int8_t y, uint8_t w);
void draw_horz_line(int8_t x, int8_t y, uint8_t w);
void draw_bitmap(uint8_t *fb, uint8_t x, uint8_t y, const struct Bitmap *bitmap);

#ifdef __cplusplus
}
#endif