#include <stdio.h>
#include <string.h>

#include "civetweb.h"
#include "stockc/market.h"


static int market_history_handler(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *req = mg_get_request_info(conn);

    const char *symbol = "UNKNOWN";

    if (req->query_string) {
        static char sym[32];
        mg_get_var(req->query_string, strlen(req->query_string),
                   "symbol", sym, sizeof(sym));
        if (*sym) symbol = sym;
    }

    char response[256];
    snprintf(response, sizeof(response),
        "{"
        "\"symbol\":\"%s\","
        "\"prices\":[123.4,124.1,125.9]"
        "}",
        symbol);

    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        strlen(response),
        response);

    return 200;
}


void register_market_routes(struct mg_context *ctx)
{
    mg_set_request_handler(ctx, "/api/market/history", market_history_handler, NULL);
}
