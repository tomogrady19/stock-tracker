#include <stdio.h>
#include <string.h>

#include "civetweb.h"
#include "stockc/market.h"
#include "stockc/alpha_vantage.h"
#include "../cache/history_cache.h"


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


// ----- History handler (using cache module) -----

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

    // ----- Cache hit -----
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

    // ----- Cache miss → fetch real data -----
    char json[8192];

    int rc = alpha_vantage_get_daily_history_json(
        symbol,
        json,
        sizeof(json)
    );

    if (rc != 0) {
        mg_printf(conn,
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: application/json\r\n"
        );
        add_cors_headers(conn);
        mg_printf(conn, "\r\n{\"error\":\"failed to fetch history\"}");
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