#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

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

EventGroupHandle_t gpio_init();

bool get_digital_value(gpio_output gpio);
void set_digital_value(gpio_output gpio, bool value);

uint16_t get_analog_value();
bool get_digital_input_value();
bool up_button_pressed();
