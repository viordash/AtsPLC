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
bool storage_0_exists(const char * name) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s", storage_0_path, name);
    struct stat st;
    return stat(filename, &st) == 0;
}
bool storage_1_exists(const char * name) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s", storage_1_path, name);
    struct stat st;
    return stat(filename, &st) == 0;
}

void create_backups_storage() {
    mkdir(backups_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}
void remove_backups_storage() {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", backups_path);
    system(cmd);
}
bool backups_storage_exists(const char * name) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s", backups_path, name);
    struct stat st;
    return stat(filename, &st) == 0;
}