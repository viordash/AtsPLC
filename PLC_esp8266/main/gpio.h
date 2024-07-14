#pragma once

#include "freertos/event_groups.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#define BUTTON_UP_IO_CLOSE BIT0
#define BUTTON_UP_IO_OPEN BIT1
#define BUTTON_DOWN_IO_CLOSE BIT2
#define BUTTON_DOWN_IO_OPEN BIT3
#define BUTTON_RIGHT_IO_CLOSE BIT4
#define BUTTON_RIGHT_IO_OPEN BIT5
#define BUTTON_SELECT_IO_CLOSE BIT6
#define BUTTON_SELECT_IO_OPEN BIT7
#define INPUT_1_IO_CLOSE BIT8
#define INPUT_1_IO_OPEN BIT9

typedef enum { OUTPUT_0 = 0x01, OUTPUT_1 = 0x02 } gpio_output;

EventGroupHandle_t gpio_init(uint32_t startup_state);
bool get_digital_value(gpio_output gpio);
void set_digital_value(gpio_output gpio, bool value);
uint16_t get_analog_value();
bool select_button_pressed();