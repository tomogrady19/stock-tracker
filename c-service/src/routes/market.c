#include <stdio.h>
#include <string.h>

#include "civetweb.h"
#include "stockc/market.h"


int market_get_quote(const char *symbol, struct stock_quote *out)
{
    if (!symbol || !out) return -1;

    strncpy(out->symbol, symbol, sizeof(out->symbol) - 1);

    // Stub values (fake for now)
    out->price = 123.45;
    out->change = 1.23;
    out->change_percent = 1.01;

    return 0;
}


int handle_market_quote(struct mg_connection *conn, void *cbdata)
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
            "Content-Type: application/json\r\n\r\n"
            "{\"error\":\"symbol parameter required\"}");
        return 1;
    }

    struct stock_quote q;

    if (market_get_quote(symbol, &q) != 0) {
        mg_printf(conn,
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: application/json\r\n\r\n"
            "{\"error\":\"failed to fetch quote\"}");
        return 1;
    }

    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n\r\n"
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


void register_market_routes(struct mg_context *ctx)
{
    mg_set_request_handler(ctx,
        "/api/market/quote",
        handle_market_quote,
        NULL);
}
