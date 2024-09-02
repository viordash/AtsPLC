#include "LogicProgram/Serializer/LogicElementFactory.h"
#include "LogicProgram/Network.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_LogicElementFactory = "LogicElementFactory";

LogicElement *LogicElementFactory::Create(TvElementType element_type) {
    ESP_LOGI(TAG_LogicElementFactory, "Create: element type:%u", element_type);

    LogicElement *element = NULL;

    switch (element_type) {
        case et_Network:
            element = new Network();
            break;

        case et_InputNC:
            break;

        default:
            break;
    }
    return element;
}
