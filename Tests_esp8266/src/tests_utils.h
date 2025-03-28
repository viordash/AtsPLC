#pragma once

#include "partitions.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

void create_storage_0();
void create_storage_1();
void remove_storage_0();
void remove_storage_1();
bool storage_0_exists(const char *name);
bool storage_1_exists(const char *name);

void create_backups_storage();
void remove_backups_storage();
bool backups_storage_exists(const char *name);

#ifdef __cplusplus
}
#endif