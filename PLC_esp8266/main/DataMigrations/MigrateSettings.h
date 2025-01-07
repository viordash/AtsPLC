#pragma once

#include "MigrateAnyData/MigrateAnyData.h"
#include "Settings_Initial.h"
#include "Settings_20250107.h"

const TDataMigrate Migrate_Settings[] = {
    { INITIAL_VERSION, 0, 0, 0 },
    MigrateSettings::Initial::DataMigrate,   //
    MigrateSettings::v20250107::DataMigrate,   //
};

const TDataMigrateItems SettingsMigrations = {
    Migrate_Settings,                                          //
    sizeof(Migrate_Settings) / sizeof(Migrate_Settings[0]) //
};
