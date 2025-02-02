#pragma once

#include "DataMigrations/MigrateSettings.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

extern CurrentSettings::device_settings settings;

void load_settings();
void store_settings();

void lock_settings();
void unlock_settings();

#define SAFETY_SETTINGS(action)                                                                    \
    {                                                                                              \
        lock_settings();                                                                           \
        action unlock_settings();                                                                  \
    }
