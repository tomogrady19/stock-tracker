#include "stockc/alpha_vantage.h"
#include "stockc/http_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yyjson.h"

#define HISTORY_DAYS 100


// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

static const char *get_api_key(void)
{
    return getenv("STOCKC_ALPHA_VANTAGE_KEY");
}

static int parse_percent(const char *s, double *out)
{
    char buf[32];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *pct = strchr(buf, '%');
    if (pct) *pct = '\0';

    *out = atof(buf);
    return 0;
}

static void log_api_call(const char *endpoint, const char *symbol)
{
    fprintf(stderr,
        "[alpha_vantage] API call: %s (symbol=%s)\n",
        endpoint,
        symbol
    );
}

static void log_api_message(const char *endpoint, const char *type, const char *msg)
{
    fprintf(stderr,
        "[alpha_vantage] %s response (%s): %s\n",
        endpoint,
        type,
        msg
    );
}


// ------------------------------------------------------------
// Quote
// ------------------------------------------------------------

int alpha_vantage_get_quote(const char *symbol, struct stock_quote *out)
{
    if (!symbol || !out)
        return -1;

    const char *api_key = get_api_key();
    if (!api_key)
        return -2;

    log_api_call("GLOBAL_QUOTE", symbol);

    char url[512];
    snprintf(
        url, sizeof(url),
        "https://www.alphavantage.co/query"
        "?function=GLOBAL_QUOTE"
        "&symbol=%s"
        "&apikey=%s",
        symbol, api_key
    );

    struct http_response res;
    if (http_get(url, 10000, &res) != 0)
        return -3;

    yyjson_doc *doc = yyjson_read(res.body, res.size, 0);
    http_response_free(&res);

    if (!doc)
        return -4;

    yyjson_val *root = yyjson_doc_get_root(doc);

    yyjson_val *note = yyjson_obj_get(root, "Note");
    yyjson_val *err  = yyjson_obj_get(root, "Error Message");
    yyjson_val *info = yyjson_obj_get(root, "Information");

    if (note || err || info) {
        const char *msg =
            note ? yyjson_get_str(note) :
            err  ? yyjson_get_str(err)  :
                   yyjson_get_str(info);

        log_api_message("GLOBAL_QUOTE", "info/error", msg ? msg : "(no message)");
        yyjson_doc_free(doc);
        return -100;
    }

    yyjson_val *quote = yyjson_obj_get(root, "Global Quote");
    if (!quote) {
        yyjson_doc_free(doc);
        return -5;
    }

    const char *price_s =
        yyjson_get_str(yyjson_obj_get(quote, "05. price"));
    const char *change_s =
        yyjson_get_str(yyjson_obj_get(quote, "09. change"));
    const char *pct_s =
        yyjson_get_str(yyjson_obj_get(quote, "10. change percent"));

    if (!price_s || !change_s || !pct_s) {
        yyjson_doc_free(doc);
        return -6;
    }

    out->price = atof(price_s);
    out->change = atof(change_s);
    parse_percent(pct_s, &out->change_percent);
    strncpy(out->symbol, symbol, sizeof(out->symbol) - 1);

    yyjson_doc_free(doc);
    return 0;
}


// ------------------------------------------------------------
// Daily history
// ------------------------------------------------------------

int alpha_vantage_get_daily_history_json(
    const char *symbol,
    char *out_json,
    size_t out_size
)
{
    if (!symbol || !out_json || out_size == 0)
        return -1;

    const char *api_key = get_api_key();
    if (!api_key)
        return -2;

    log_api_call("TIME_SERIES_DAILY", symbol);

    char url[512];
    snprintf(
        url, sizeof(url),
        "https://www.alphavantage.co/query"
        "?function=TIME_SERIES_DAILY" //ADJUSTED is premium only, but we only need close price so regular is fine
        "&symbol=%s"
        "&apikey=%s",
        symbol, api_key
    );

    struct http_response res;
    if (http_get(url, 10000, &res) != 0)
        return -3;

    yyjson_doc *doc = yyjson_read(res.body, res.size, 0);
    http_response_free(&res);

    if (!doc)
        return -4;

    yyjson_val *root = yyjson_doc_get_root(doc);

    yyjson_val *note = yyjson_obj_get(root, "Note");
    yyjson_val *err  = yyjson_obj_get(root, "Error Message");
    yyjson_val *info = yyjson_obj_get(root, "Information");

    if (note || err || info) {
        const char *msg =
            note ? yyjson_get_str(note) :
            err  ? yyjson_get_str(err)  :
                   yyjson_get_str(info);

        log_api_message("TIME_SERIES_DAILY", "info/error", msg ? msg : "(no message)");
        yyjson_doc_free(doc);
        return -100;
    }

    yyjson_val *series =
        yyjson_obj_get(root, "Time Series (Daily)");

    if (!series || !yyjson_is_obj(series)) {
        yyjson_doc_free(doc);
        return -5;
    }

    size_t written = 0;
    written += snprintf(
        out_json + written,
        out_size - written,
        "{"
          "\"symbol\":\"%s\","
          "\"series\":[",
        symbol
    );

    size_t count = 0;
    yyjson_obj_iter iter;
    yyjson_obj_iter_init(series, &iter);

    yyjson_val *key, *val;

    while ((key = yyjson_obj_iter_next(&iter)) &&
           (val = yyjson_obj_iter_get_val(key)) &&
           count < HISTORY_DAYS)
    {
        const char *date = yyjson_get_str(key);
        yyjson_val *close =
            yyjson_obj_get(val, "4. close");

        if (!date || !close)
            continue;

        const char *price_s = yyjson_get_str(close);
        if (!price_s)
            continue;

        double price = atof(price_s);

        written += snprintf(
            out_json + written,
            out_size - written,
            "%s{\"date\":\"%s\",\"price\":%.2f}",
            (count == 0 ? "" : ","),
            date,
            price
        );

        count++;
    }

    snprintf(
        out_json + written,
        out_size - written,
        "]}"
    );

    yyjson_doc_free(doc);
    return 0;
}