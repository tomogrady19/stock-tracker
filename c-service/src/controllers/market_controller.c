#include <string.h>
#include <stdlib.h>

#include "market_controller.h"
#include "stockc/market.h"
#include "../cache/history_cache.h"


// ============================================================
// DEV fallback history
// ============================================================

static const char *dev_fallback_history =
"{"
  "\"symbol\":\"AAPL\","
  "\"series\":["
    "{\"date\":\"2026-01-27\",\"price\":252.10},"
    "{\"date\":\"2026-01-28\",\"price\":258.27},"
    "{\"date\":\"2026-01-29\",\"price\":256.44},"
    "{\"date\":\"2026-01-30\",\"price\":260.05},"
    "{\"date\":\"2026-02-02\",\"price\":259.40}"
  "]"
"}";


// ============================================================
// Domain helpers
// ============================================================

static int extract_latest_quote(const char *history_json,
                                struct stock_quote *out)
{
    const char *p = strstr(history_json, "\"price\":");
    if (!p) return -1;

    double latest = atof(p + 8);

    const char *p2 = strstr(p + 8, "\"price\":");
    if (!p2) return -1;

    double previous = atof(p2 + 8);

    out->price = latest;
    out->change = latest - previous;
    out->change_percent =
        (out->change / previous) * 100.0;

    return 0;
}


// ============================================================
// Controllers
// ============================================================

int market_quote_controller(struct mg_connection *conn,
                            const char *symbol)
{
    const char *history = history_cache_get(symbol);

    if (!history) {
        char json[8192];

        int rc = alpha_vantage_get_daily_history_json(
            symbol,
            json,
            sizeof(json)
        );

        if (rc == 0) {
            history_cache_set(symbol, json);
            history = history_cache_get(symbol);
        } else {
            history = dev_fallback_history;
        }
    }

    struct stock_quote q;
    memset(&q, 0, sizeof(q));
    strncpy(q.symbol, symbol, sizeof(q.symbol) - 1);

    if (extract_latest_quote(history, &q) != 0) {
        q.price = 0.0;
        q.change = 0.0;
        q.change_percent = 0.0;
    }

    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
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
                              const char *symbol)
{
    const char *cached = history_cache_get(symbol);
    if (cached) {
        mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "\r\n%s",
            cached
        );
        return 1;
    }

    char json[8192];

    int rc = alpha_vantage_get_daily_history_json(
        symbol,
        json,
        sizeof(json)
    );

    if (rc != 0) {
        const char *stale = history_cache_get(symbol);
        if (stale) {
            mg_printf(conn,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "\r\n%s",
                stale
            );
            return 1;
        }

        mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "\r\n%s",
            dev_fallback_history
        );
        return 1;
    }

    history_cache_set(symbol, json);

    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "\r\n%s",
        json
    );

    return 1;
}