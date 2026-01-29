#include "stockc/http_client.h"

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct mem_buf {
    char *ptr;
    size_t len;
};

static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsz = size * nmemb;
    struct mem_buf *mem = (struct mem_buf *)userp;

    char *new_ptr = (char *)realloc(mem->ptr, mem->len + realsz + 1);
    if (!new_ptr) return 0;

    mem->ptr = new_ptr;
    memcpy(mem->ptr + mem->len, contents, realsz);
    mem->len += realsz;
    mem->ptr[mem->len] = '\0';

    return realsz;
}

int http_get(const char *url, long timeout_ms, struct http_response *out) {
    if (!url || !out) return 1;
    memset(out, 0, sizeof(*out));

    static int curl_initialized = 0;
    if (!curl_initialized) {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0)
            return 2;
        curl_initialized = 1;
    }

    CURL *curl = curl_easy_init();
    if (!curl) return 3;

    struct mem_buf mem = {0};

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mem);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "stockc/1.0");

    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        curl_easy_cleanup(curl);
        free(mem.ptr);
        return 4;
    }

    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);

    curl_easy_cleanup(curl);

    out->status = status;
    out->body = mem.ptr;
    out->size = mem.len;

    return 0;
}

void http_response_free(struct http_response *res) {
    if (!res) return;
    free(res->body);
    res->body = NULL;
    res->size = 0;
    res->status = 0;
}
