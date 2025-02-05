#pragma once

#include <stdint.h>
#include <unistd.h>

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define INCOME_RAIL_TOP 17
#define INCOME_RAIL_HEIGHT 25
#define INCOME_RAIL_NETWORK_TOP (INCOME_RAIL_HEIGHT / 2)
#define INCOME_RAIL_WIDTH 2
#define OUTCOME_RAIL_TOP (INCOME_RAIL_TOP)
#define OUTCOME_RAIL_RIGHT (SCROLLBAR_LEFT - 1)
#define OUTCOME_RAIL_HEIGHT (INCOME_RAIL_HEIGHT)
#define OUTCOME_RAIL_NETWORK_TOP (OUTCOME_RAIL_HEIGHT / 2)
#define OUTCOME_RAIL_WIDTH 2
#define SCROLLBAR_TOP (INCOME_RAIL_TOP)
#define SCROLLBAR_WIDTH 2
#define SCROLLBAR_HEIGHT (DISPLAY_HEIGHT - SCROLLBAR_TOP)
#define SCROLLBAR_LEFT (DISPLAY_WIDTH - SCROLLBAR_WIDTH)
#define VERT_PROGRESS_BAR_HEIGHT 10
#define VERT_PROGRESS_BAR_WIDTH 3
#define HORZ_PROGRESS_BAR_HEIGHT 3
#define HORZ_PROGRESS_BAR_WIDTH 10
#define WIRE_BLINK_BODY_WIDTH 8
#define WIRE_STANDART_WIDTH (16 + WIRE_BLINK_BODY_WIDTH)

#include "Display/Common.h"

void display_init();

uint8_t *begin_render();
void end_render(uint8_t *fb);
int draw_text_f4X7(uint8_t *fb, uint8_t x, uint8_t y, const char *text);
int draw_text_f5X7(uint8_t *fb, uint8_t x, uint8_t y, const char *text);
int draw_text_f6X12(uint8_t *fb, uint8_t x, uint8_t y, const char *text);
int draw_text_f6X12_colored(uint8_t *fb, uint8_t x, uint8_t y, const char *text, bool inverse);
int draw_text_f8X14(uint8_t *fb, uint8_t x, uint8_t y, const char *text);

int get_text_f4X7_height();
int get_text_f5X7_height();
int get_text_f6X12_height();
int get_text_f8X14_height();

bool draw_active_network(uint8_t *fb, uint8_t x, uint8_t y, uint8_t w);
bool draw_passive_network(uint8_t *fb, uint8_t x, uint8_t y, uint8_t w, bool inverse_dash);
bool draw_active_income_rail(uint8_t *fb, uint8_t x, uint8_t y);
bool draw_passive_income_rail(uint8_t *fb, uint8_t x, uint8_t y);
bool draw_outcome_rail(uint8_t *fb, uint8_t x, uint8_t y);
bool draw_vert_progress_bar(uint8_t *fb, uint8_t x, uint8_t y, uint8_t percent04);
bool draw_horz_progress_bar(uint8_t *fb, uint8_t x, uint8_t y, uint8_t percent04);
bool draw_vert_line(uint8_t *fb, uint8_t x, uint8_t y, uint8_t w);
bool draw_horz_line(uint8_t *fb, uint8_t x, uint8_t y, uint8_t w);
bool draw_rectangle(uint8_t *fb, int8_t x, int8_t y, uint8_t w, uint8_t h);
void draw_bitmap(uint8_t *fb, uint8_t x, uint8_t y, const struct Bitmap *bitmap);
