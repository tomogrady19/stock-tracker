#include <string.h>
#include <time.h>

#include "market_controller.h"
#include "../services/market_service.h"
#include "../http/cors.h"
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

    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
    );
    add_cors_headers(conn);
    mg_printf(conn,
        "\r\n"
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

    return 1;
}


int market_history_controller(struct mg_connection *conn,
                              const char *symbol,
                              int days)
{
    struct market_history_result res =
        market_service_get_history(symbol, days);

    const char *source_str = source_to_string(res.source);

    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
    );
    add_cors_headers(conn);

    /*
     * We wrap the original history JSON inside a new object
     * so the frontend can see:
     *   - source: "live" | "cache" | "demo"
     *   - fetchedAt: unix timestamp
     *
     * Result shape:
     * {
     *   "symbol": "AAPL",
     *   "source": "cache",
     *   "fetchedAt": 1707152030,
     *   "series": [...]
     * }
     *
     * We achieve this by removing the leading '{' from the
     * original JSON and prepending our own fields.
     */

    const char *inner = res.json;

    if (inner && inner[0] == '{')
        inner++;

    mg_printf(conn,
        "\r\n"
        "{"
          "\"source\":\"%s\","
          "\"fetchedAt\":%lld,"
          "%s",
        source_str,
        (long long)res.fetched_at,
        inner ? inner : "\"series\":[]}"
    );

    return 1;
}