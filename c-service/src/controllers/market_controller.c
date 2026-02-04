#include <string.h>

#include "market_controller.h"
#include "../services/market_service.h"
#include "../http/cors.h"
#include "stockc/market.h"


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
                              const char *symbol)
{
    const char *history = market_service_get_history(symbol);

    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
    );
    add_cors_headers(conn);
    mg_printf(conn,
        "\r\n%s",
        history
    );

    return 1;
}