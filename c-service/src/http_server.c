#include <stdio.h>
#include <string.h>

#include "civetweb.h"
#include "stockc/http.h"
#include "stockc/market.h"

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

    return 200;
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
    register_market_routes(ctx);

    printf("stockc listening on http://localhost:%d\n", port);

    while (1) {
        sleep_ms(1000);
    }

    mg_stop(ctx);
    return 0;
}
