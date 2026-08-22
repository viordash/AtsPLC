#include "UriHelper.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

const char *UriHelper::GetPathFromUri(char *dest, const char *uri, size_t destSize) {
    size_t pathlen = strlen(uri);
    bool isIndexPath = pathlen == 0;
    if (isIndexPath) {
        uri = "index.html";
        pathlen = strlen(uri);
    }

    const char *quest = strchr(uri, '?');
    if (quest != NULL) {
        pathlen = MIN(pathlen, (size_t)(quest - uri));
    }
    const char *hash = strchr(uri, '#');
    if (hash != NULL) {
        pathlen = MIN(pathlen, (size_t)(hash - uri));
    }

    if (pathlen + 1 > destSize) {
        return NULL;
    }

    strncpy(dest, uri, pathlen + 1);
    dest[pathlen] = 0;

    return dest;
}

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

bool UriHelper::GetPathParameterU32(const char *pattern, const char *uri, uint32_t *outVal) {
    if (pattern == NULL || uri == NULL || outVal == NULL) {
        return false;
    }
    const char *pos = strstr(uri, pattern);
    if (pos == NULL) {
        return false;
    }
    const char *tail = pos + strlen(pattern);

    char *endptr;
    errno = 0;
    uint32_t converted = strtoul(tail, &endptr, 10);

    if ((errno == ERANGE && converted == ULONG_MAX) || (errno != 0 && converted == 0)) {
        return false;
    }
    if (endptr == tail) {
        return false;
    }
    *outVal = converted;
    return true;
}
