#include "LogicProgram/Serializer/LogicElementFactory.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_LogicElementFactory = "LogicElementFactory";

LogicElement *LogicElementFactory::Create(uint8_t *buffer, size_t buffer_size) {
    ESP_LOGI(TAG_LogicElementSerializer, "Serialize: buffer:%p, size:%u", buffer, buffer_size);

    size_t readed = 0;
    LogicElement *element = NULL;
    TvElement tvElement;
    if (!LogicElement::ReadRecord(&tvElement, sizeof(tvElement), buffer, buffer_size, &readed)) {
        return element;
    }

    switch (tvElement.type) {
        case et_Network:
            // element = new
            break;

        default:
            break;
    }
    return element;
}
