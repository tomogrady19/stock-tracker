#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Simple owned buffer returned by HTTP GET.
struct http_response {
    long status;     // HTTP status code (e.g., 200). 0 if request failed before getting one.
    char *body;      // malloc'd, null-terminated (may be NULL on failure)
    size_t size;     // bytes in body (excluding null terminator)
};

// Performs a GET request.
// Returns 0 on success (even if status != 200), non-zero on transport/setup error.
// Caller must call http_response_free().
int http_get(const char *url, long timeout_ms, struct http_response *out);

void http_response_free(struct http_response *res);

#ifdef __cplusplus
}
#endif
