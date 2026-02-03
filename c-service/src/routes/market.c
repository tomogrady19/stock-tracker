#include <stdio.h>
#include <string.h>

#include "civetweb.h"
#include "stockc/market.h"
#include "stockc/alpha_vantage.h"


static void add_cors_headers(struct mg_connection *conn)
{
    mg_printf(conn,
        "Access-Control-Allow-Origin: http://localhost:5173\r\n"
        "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
    );
}


// ----- Quote -----

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
        mg_printf(conn,
            "\r\n"
            "{\"error\":\"symbol parameter required\"}");
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
        mg_printf(conn,
            "\r\n"
            "{\"error\":\"%s\"}",
            msg
        );
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


// ----- History (stubbed for now) -----
// Fixed range: last 5 trading days (static sample data).
// Next step will replace this with Alpha Vantage TIME_SERIES_DAILY parsing.

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
        mg_printf(conn,
            "\r\n"
            "{\"error\":\"symbol parameter required\"}");
        return 1;
    }

    // Stub data (dates/values are just placeholders).
    // Shape is what matters for now.
    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
    );
    add_cors_headers(conn);
    mg_printf(conn,
        "\r\n"
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

    return 1;
}


// ----- Route registration -----

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
