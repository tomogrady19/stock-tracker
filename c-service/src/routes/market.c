#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "civetweb.h"
#include "stockc/market.h"
#include "../cache/history_cache.h"

// ----- DEV fallback history -----

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


// ----- CORS -----

static void add_cors_headers(struct mg_connection *conn)
{
    mg_printf(conn,
        "Access-Control-Allow-Origin: http://localhost:5173\r\n"
        "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
    );
}


// ----- Helpers -----

static int extract_latest_quote(
    const char *history_json,
    struct stock_quote *out
)
{
    // VERY simple parsing:
    // We assume the first two entries are the most recent days
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


// ----- Quote handler (derived from history) -----

static int handle_market_quote(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *req = mg_get_request_info(conn);

    // CORS preflight
    if (strcmp(req->request_method, "OPTIONS") == 0) {
        mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 0\r\n"
        );
        add_cors_headers(conn);
        mg_printf(conn, "\r\n");
        return 1;
    }

    char symbol[16] = {0};

    if (req->query_string) {
        mg_get_var(
            req->query_string,
            strlen(req->query_string),
            "symbol",
            symbol,
            sizeof(symbol)
        );
    }

    if (strlen(symbol) == 0) {
        mg_printf(conn,
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: application/json\r\n"
        );
        add_cors_headers(conn);
        mg_printf(conn,
            "\r\n{\"error\":\"symbol parameter required\"}");
        return 1;
    }

    const char *history = history_cache_get(symbol);

    if (!history) {
        // Try to fetch history
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
            // Final fallback — dev history
            history = dev_fallback_history;
        }
    }

    struct stock_quote q;
    strncpy(q.symbol, symbol, sizeof(q.symbol) - 1);

    // Derive quote from history (or fallback)
    if (extract_latest_quote(history, &q) != 0) {
        // Absolute last resort: safe defaults
        q.price = 0.0;
        q.change = 0.0;
        q.change_percent = 0.0;
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


// ----- History handler -----

static int handle_market_history(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *req = mg_get_request_info(conn);

    // CORS preflight
    if (strcmp(req->request_method, "OPTIONS") == 0) {
        mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 0\r\n"
        );
        add_cors_headers(conn);
        mg_printf(conn, "\r\n");
        return 1;
    }

    char symbol[16] = {0};

    if (req->query_string) {
        mg_get_var(req->query_string,
                   strlen(req->query_string),
                   "symbol",
                   symbol,
                   sizeof(symbol));
    }

    if (strlen(symbol) == 0) {
        mg_printf(conn,
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: application/json\r\n"
        );
        add_cors_headers(conn);
        mg_printf(conn, "\r\n{\"error\":\"symbol parameter required\"}");
        return 1;
    }

    const char *cached = history_cache_get(symbol);
    if (cached) {
        mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
        );
        add_cors_headers(conn);
        mg_printf(conn, "\r\n%s", cached);
        return 1;
    }

    // Cache miss → fetch history
    char json[8192];

    int rc = alpha_vantage_get_daily_history_json(
        symbol,
        json,
        sizeof(json)
    );

    if (rc != 0) {
        // Alpha Vantage failed — serve stale cache if available
        const char *stale = history_cache_get(symbol);
        if (stale) {
            mg_printf(conn,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
            );
            add_cors_headers(conn);
            mg_printf(conn, "\r\n%s", stale);
            return 1;
        }

        // DEV fallback (cold start + upstream unavailable)
        mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
        );
        add_cors_headers(conn);
        mg_printf(conn, "\r\n%s", dev_fallback_history);
        return 1;
    }


    history_cache_set(symbol, json);

    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
    );
    add_cors_headers(conn);
    mg_printf(conn, "\r\n%s", json);

    return 1;
}


// ----- Routes -----

void register_market_routes(struct mg_context *ctx)
{
    mg_set_request_handler(ctx,
        "/api/market/quote",
        handle_market_quote,
        NULL);

    mg_set_request_handler(ctx,
        "/api/market/history",
        handle_market_history,
        NULL);
}