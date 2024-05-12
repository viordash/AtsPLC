#include "esp_err.h"
#include <stdint.h>
#include <unistd.h>

/**
 * @brief ADC working mode enumeration
 */
typedef enum { ADC_READ_TOUT_MODE = 0, ADC_READ_VDD_MODE, ADC_READ_MAX_MODE } adc_mode_t;

/**
 * @brief ADC initialization parameter structure type definition
 */
typedef struct {
    adc_mode_t mode; /*!< ADC mode */
    uint8_t clk_div; /*!< ADC sample collection clock=80M/clk_div, range[8, 32] */
} adc_config_t;

esp_err_t adc_init(adc_config_t *config);

esp_err_t adc_read(uint16_t *data);