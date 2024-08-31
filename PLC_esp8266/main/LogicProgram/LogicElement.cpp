#include "LogicProgram/LogicElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LogicElement::LogicElement() {
    this->state = LogicItemState::lisPassive;
}

LogicElement::~LogicElement() {
}

bool LogicElement::WriteRecord(void *data,
                               size_t data_size,
                               uint8_t *buffer,
                               size_t buffer_size,
                               size_t *writed) {
    if (buffer_size - *writed < data_size) {
        return false;
    }
    bool just_obtain_size = buffer == NULL;
    if (!just_obtain_size) {
        memcpy(&buffer[*writed], data, data_size);
    }
    *writed += data_size;
    return true;
}

bool LogicElement::ReadRecord(void *data,
                              size_t data_size,
                              uint8_t *buffer,
                              size_t buffer_size,
                              size_t *readed) {
    if (buffer_size - *readed < data_size) {
        return false;
    }
    memcpy(data, &buffer[*readed], data_size);
    *readed += data_size;
    return true;
}