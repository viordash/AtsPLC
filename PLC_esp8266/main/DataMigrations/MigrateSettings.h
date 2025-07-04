#pragma once

#include "Settings_Initial.h"
#include "Settings_20250107.h"
#include "Settings_20250209.h"
#include "Settings_20250413.h"
#include "Settings_20250619.h"

const TDataMigrate Migrate_Settings[] = {
    { INITIAL_VERSION, 0, 0, 0 },
    MigrateSettings::Initial::DataMigrate,   //
    MigrateSettings::v20250107::DataMigrate, //
    MigrateSettings::v20250209::DataMigrate, //
    MigrateSettings::v20250413::DataMigrate, //
    MigrateSettings::v20250619::DataMigrate, //
};

const TDataMigrateItems SettingsMigrations = {
    Migrate_Settings,                                      //
    sizeof(Migrate_Settings) / sizeof(Migrate_Settings[0]) //
};

namespace CurrentSettings = MigrateSettings::v20250619::Snapshot;