#include "tests_utils.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

void create_storage_0() {
    mkdir(storage_0_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}
void create_storage_1() {
    mkdir(storage_1_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}
void remove_storage_0() {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", storage_0_path);
    system(cmd);
}
void remove_storage_1() {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", storage_1_path);
    system(cmd);
}
bool storage_0_exists() {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s", storage_0_path, settings_storage_name);
    struct stat st;
    return stat(filename, &st) == 0;
}
bool storage_1_exists() {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s", storage_1_path, settings_storage_name);
    struct stat st;
    return stat(filename, &st) == 0;
}