#include "stockc/market.h"
#include "stockc/http_client.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "yyjson.h"

/*
  Reads Alpha Vantage API key from environment variable:

    STOCKC_ALPHA_VANTAGE_KEY

  Example (PowerShell):
    setx STOCKC_ALPHA_VANTAGE_KEY "YOUR_KEY_HERE"

  Example (cmd.exe):
    setx STOCKC_ALPHA_VANTAGE_KEY "YOUR_KEY_HERE"
*/

static const char *get_api_key(void) {
    const char *key = getenv("STOCKC_ALPHA_VANTAGE_KEY");
    if (!key || key[0] == '\0') return NULL;
    return key;
}

// Alpha Vantage returns percent like "1.2345%". Convert to double 1.2345 (not 0.012345).
static int parse_percent(const char *s, double *out) {
    if (!s || !out) return 1;

    char buf[64];
    size_t n = strlen(s);
    if (n == 0 || n >= sizeof(buf)) return 2;

    memcpy(buf, s, n + 1);
    char *pct = strchr(buf, '%');
    if (pct) *pct = '\0';

    char *end = NULL;
    double v = strtod(buf, &end);
    if (end == buf) return 3;

    *out = v;
    return 0;
}

// Extract a string field safely from an object.
static const char *obj_get_str(yyjson_val *obj, const char *key) {
    yyjson_val *v = yyjson_obj_get(obj, key);
    if (!v) return NULL;
    return yyjson_get_str(v);
}

/*
  Fetches a live quote from Alpha Vantage GLOBAL_QUOTE endpoint.

  Returns:
    0  success
    1  invalid args
    2  missing API key
    3  HTTP transport failure
    4  HTTP returned non-200
    5  empty response
    6  JSON parse failure
    7  upstream error/rate limit message
    8  missing expected fields
    9  numeric conversion failure
*/
int alpha_vantage_get_quote(const char *symbol, struct stock_quote *out) {
    if (!symbol || !out) return 1;

    const char *key = get_api_key();
    if (!key) return 2;

    // Build URL (keep simple; typical symbols are safe without encoding).
    char url[512];
    snprintf(
        url, sizeof(url),
        "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=%s&apikey=%s",
        symbol, key
    );

    struct http_response res;
    int rc = http_get(url, 8000, &res);
    if (rc != 0) return 3;

    if (res.status != 200) {
        http_response_free(&res);
        return 4;
    }

    if (!res.body || res.size == 0) {
        http_response_free(&res);
        return 5;
    }

    yyjson_doc *doc = yyjson_read(res.body, res.size, 0);
    if (!doc) {
        http_response_free(&res);
        return 6;
    }

    yyjson_val *root = yyjson_doc_get_root(doc);

    // Alpha Vantage may respond with:
    // { "Note": "..." } (rate limiting)
    // { "Error Message": "..." } (bad symbol / params)
    // { "Information": "..." } (other)
    if (yyjson_obj_get(root, "Note") ||
        yyjson_obj_get(root, "Error Message") ||
        yyjson_obj_get(root, "Information")) {
        yyjson_doc_free(doc);
        http_response_free(&res);
        return 7;
    }

    yyjson_val *gq = yyjson_obj_get(root, "Global Quote");
    if (!gq || !yyjson_is_obj(gq)) {
        yyjson_doc_free(doc);
        http_response_free(&res);
        return 8;
    }

    const char *sym = obj_get_str(gq, "01. symbol");
    const char *price_s = obj_get_str(gq, "05. price");
    const char *change_s = obj_get_str(gq, "09. change");
    const char *pct_s = obj_get_str(gq, "10. change percent");

    if (!sym || !price_s || !change_s || !pct_s) {
        yyjson_doc_free(doc);
        http_response_free(&res);
        return 8;
    }

    char *end = NULL;
    double price = strtod(price_s, &end);
    if (end == price_s) {
        yyjson_doc_free(doc);
        http_response_free(&res);
        return 9;
    }

    end = NULL;
    double change = strtod(change_s, &end);
    if (end == change_s) {
        yyjson_doc_free(doc);
        http_response_free(&res);
        return 9;
    }

    double pct = 0.0;
    if (parse_percent(pct_s, &pct) != 0) {
        yyjson_doc_free(doc);
        http_response_free(&res);
        return 9;
    }

    memset(out, 0, sizeof(*out));
    snprintf(out->symbol, sizeof(out->symbol), "%s", sym);
    out->price = price;
    out->change = change;
    out->change_percent = pct;

    yyjson_doc_free(doc);
    http_response_free(&res);
    return 0;
}
