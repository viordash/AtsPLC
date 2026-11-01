#pragma once

#include "redundant_storage.h"
#include <stdint.h>
#include <unistd.h>

typedef redundant_storage backups_storage;

bool backups_storage_load(const char *name, backups_storage *storage);
void backups_storage_store(const char *name, backups_storage *storage);
void backups_storage_delete(const char *name);
