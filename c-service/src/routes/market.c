#include <stdio.h>
#include <string.h>

#include "civetweb.h"
#include "stockc/market.h"
#include "stockc/alpha_vantage.h"


int market_get_quote(const char *symbol, struct stock_quote *out)
{
    if (!symbol || !out) return -1;

    return alpha_vantage_get_quote(symbol, out);
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

    int rc = market_get_quote(symbol, &q);
    if (rc != 0) {
        int status = 500;
        const char *msg = "failed to fetch quote";

        // Better error mapping for API issues
        if (rc == 2) {
            msg = "missing api key";
        } else if (rc == 7) {
            status = 429;
            msg = "rate limit exceeded";
        }

        mg_printf(conn,
            "HTTP/1.1 %d Error\r\n"
            "Content-Type: application/json\r\n\r\n"
            "{\"error\":\"%s\"}",
            status, msg);

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
