#include "UriHelper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *UriHelper::GetContentType(const char *filename) {
#define IS_FILE_EXT(filename, ext) (strcasecmp(&filename[sLen - sizeof(ext) + 1], ext) == 0)
    size_t sLen = strlen(filename);

    if (IS_FILE_EXT(filename, ".js")) {
        return "text/javascript";
    } else if (IS_FILE_EXT(filename, ".css")) {
        return "text/css";
    } else if (IS_FILE_EXT(filename, ".pdf")) {
        return "application/pdf";
    } else if (IS_FILE_EXT(filename, ".html")) {
        return "text/html";
    } else if (IS_FILE_EXT(filename, ".jpeg")) {
        return "image/jpeg";
    } else if (IS_FILE_EXT(filename, ".ico")) {
        return "image/x-icon";
    }
    return "text/plain";
}
