#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "market_controller.h"
#include "../services/market_service.h"
#include "../http/responses.h"
#include "stockc/market.h"


// ------------------------------------------------------------
// Helper: convert enum to string for JSON
// ------------------------------------------------------------
static const char *source_to_string(enum market_data_source src)
{
    switch (src) {
        case MARKET_SOURCE_LIVE:  return "live";
        case MARKET_SOURCE_CACHE: return "cache";
        case MARKET_SOURCE_DEMO:  return "demo";
        default:                  return "demo";
    }
}


int market_quote_controller(struct mg_connection *conn,
                            const char *symbol)
{
    struct stock_quote q;

    if (market_service_get_quote(symbol, &q) != 0) {
        memset(&q, 0, sizeof(q));
        strncpy(q.symbol, symbol, sizeof(q.symbol) - 1);
    }

    char json[512];

    snprintf(json, sizeof(json),
        "{"
          "\"symbol\":\"%s\","
          "\"price\":%.2f,"
          "\"change\":%.2f,"
          "\"changePercent\":%.2f"
        "}",
        q.symbol,
        q.price,
        q.change,
        q.change_percent
    );

    send_json_response(conn, 200, json);
    return 1;
}


int market_history_controller(struct mg_connection *conn,
                              const char *symbol,
                              int days)
{
    struct market_history_result res =
        market_service_get_history(symbol, days);

    const char *source_str = source_to_string(res.source);

    const char *inner = res.json;

    if (inner && inner[0] == '{')
        inner++;

    size_t needed = strlen(inner ? inner : "\"series\":[]}")
                    + 256;

    char *json = malloc(needed);
    if (!json) {
        send_json_error(conn, 500, "memory allocation failed");
        return 1;
    }

    snprintf(json, needed,
        "{"
          "\"source\":\"%s\","
          "\"fetchedAt\":%lld,"
          "%s",
        source_str,
        (long long)res.fetched_at,
        inner ? inner : "\"series\":[]}"
    );

    send_json_response(conn, 200, json);

    free(json);
    return 1;
}