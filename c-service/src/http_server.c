#include <stdio.h>
#include <string.h>

#include "civetweb.h"
#include "stockc/http.h"
#include "stockc/market.h"

static int handle_market_quote(struct mg_connection *conn, void *cbdata);

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep((ms) * 1000)
#endif


static int health_handler(struct mg_connection *conn, void *cbdata)
{
    const char *response =
        "{"
        "\"status\":\"ok\","
        "\"service\":\"stockc\""
        "}";

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %zu\r\n"
              "Connection: close\r\n"
              "\r\n"
              "%s",
              strlen(response),
              response);

    return 1;
}


int start_http_server(int port)
{
    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);

    const char *options[] = {
        "listening_ports", port_str,
        "num_threads", "4",
        0
    };

    struct mg_callbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));

    struct mg_context *ctx = mg_start(&callbacks, NULL, options);
    if (!ctx) {
        fprintf(stderr, "Failed to start CivetWeb\n");
        return 1;
    }

    mg_set_request_handler(ctx, "/health", health_handler, NULL);
    mg_set_request_handler(ctx, "/api/market/quote", handle_market_quote, 0);
    register_market_routes(ctx);

    printf("stockc listening on http://localhost:%d\n", port);

    while (1) {
        sleep_ms(1000);
    }

    mg_stop(ctx);
    return 0;
}

static int handle_market_quote(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *req = mg_get_request_info(conn);

    char symbol[16] = {0};

    if (req->query_string) {
        mg_get_var(req->query_string, strlen(req->query_string),
                "symbol", symbol, sizeof(symbol));
    }


    if (strlen(symbol) == 0) {
        mg_printf(conn,
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: application/json\r\n\r\n"
            "{\"error\":\"symbol parameter required\"}");
        return 1;
    }

    // Stubbed values for now
    double price = 192.34;
    double change = -1.12;
    double change_percent = -0.58;

    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n\r\n"
        "{"
          "\"symbol\":\"%s\","
          "\"price\":%.2f,"
          "\"change\":%.2f,"
          "\"changePercent\":%.2f"
        "}",
        symbol, price, change, change_percent
    );

    return 1;
}
