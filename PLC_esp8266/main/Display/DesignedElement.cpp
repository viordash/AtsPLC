#include "Display/DesignedElement.h"
#include "Display/display.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DesignedElement::DesignedElement() {
    in_design_state = false;
}

void DesignedElement::BeginDesign() {
    in_design_state = true;
}

void DesignedElement::EndDesign() {
    in_design_state = false;
}
