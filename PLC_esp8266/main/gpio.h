#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef enum { OUTPUT_0 = 0x01, OUTPUT_1 = 0x02 } gpio_output;

void gpio_init(uint32_t startup_state);
bool get_digital_value(gpio_output gpio);
void set_digital_value(gpio_output gpio, bool value);
uint16_t get_analog_value();
