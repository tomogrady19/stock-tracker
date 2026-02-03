#include <stdio.h>
#include <string.h>
#include <time.h>

#include "civetweb.h"
#include "stockc/market.h"
#include "stockc/alpha_vantage.h"

#define HISTORY_CACHE_TTL 60  // seconds


// ----- CORS -----

static void add_cors_headers(struct mg_connection *conn)
{
    mg_printf(conn,
        "Access-Control-Allow-Origin: http://localhost:5173\r\n"
        "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
    );
}


// ----- Quote logic -----

int market_get_quote(const char *symbol, struct stock_quote *out)
{
    if (!symbol || !out) return -1;
    return alpha_vantage_get_quote(symbol, out);
}

static int handle_market_quote(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *req = mg_get_request_info(conn);
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

    struct stock_quote q;
    int rc = market_get_quote(symbol, &q);

    if (rc != 0) {
        int status = 500;
        const char *msg = "failed to fetch quote";

        if (rc == 2) {
            msg = "missing api key";
        } else if (rc == 7) {
            status = 429;
            msg = "rate limit exceeded";
        }

        mg_printf(conn,
            "HTTP/1.1 %d Error\r\n"
            "Content-Type: application/json\r\n",
            status
        );
        add_cors_headers(conn);
        mg_printf(conn, "\r\n{\"error\":\"%s\"}", msg);
        return 1;
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
        q.symbol, q.price, q.change, q.change_percent
    );

    return 1;
}


// ----- History cache -----

struct history_cache_entry {
    char symbol[16];
    time_t fetched_at;
    char json[2048];
};

static struct history_cache_entry history_cache = {0};

static int history_cache_valid(const char *symbol)
{
    if (strcmp(history_cache.symbol, symbol) != 0)
        return 0;

    time_t now = time(NULL);
    return difftime(now, history_cache.fetched_at) < HISTORY_CACHE_TTL;
}


// ----- History handler (cached stub) -----

static int handle_market_history(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *req = mg_get_request_info(conn);
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

    // Serve cached response if valid
    if (history_cache_valid(symbol)) {
        mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
        );
        add_cors_headers(conn);
        mg_printf(conn, "\r\n%s", history_cache.json);
        return 1;
    }

    // Populate cache (stub data for now)
    snprintf(history_cache.symbol, sizeof(history_cache.symbol), "%s", symbol);
    history_cache.fetched_at = time(NULL);

    snprintf(history_cache.json, sizeof(history_cache.json),
        "{"
          "\"symbol\":\"%s\","
          "\"series\":["
            "{\"date\":\"2026-01-27\",\"price\":252.10},"
            "{\"date\":\"2026-01-28\",\"price\":258.27},"
            "{\"date\":\"2026-01-29\",\"price\":256.44},"
            "{\"date\":\"2026-01-30\",\"price\":260.05},"
            "{\"date\":\"2026-02-02\",\"price\":259.40}"
          "]"
        "}",
        symbol
    );

    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
    );
    add_cors_headers(conn);
    mg_printf(conn, "\r\n%s", history_cache.json);

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
