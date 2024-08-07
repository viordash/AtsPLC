#include "LogicItemBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LogicItemBase::LogicItemBase(const Controller &controller) {
    this->controller = controller;
}

LogicItemBase::~LogicItemBase() {
}
