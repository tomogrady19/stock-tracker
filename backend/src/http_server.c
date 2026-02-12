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


// ---- CORS helpers ----

static void add_cors_headers(struct mg_connection *conn)
{
    mg_printf(conn,
        "Access-Control-Allow-Origin: http://localhost:5173\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Access-Control-Max-Age: 86400\r\n"
    );
}

static int options_handler(struct mg_connection *conn, void *cbdata)
{
    mg_printf(conn,
        "HTTP/1.1 204 No Content\r\n"
        "Connection: close\r\n"
    );
    add_cors_headers(conn);
    mg_printf(conn, "\r\n");
    return 1;
}


// ---- Health ----

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
        "Content-Length: %zu\r\n",
        strlen(response)
    );

    add_cors_headers(conn);

    mg_printf(conn,
        "Connection: close\r\n"
        "\r\n"
        "%s",
        response
    );

    return 1;
}


// ---- Server ----

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

    // CORS preflight handler
    mg_set_request_handler(ctx, "/**", options_handler, NULL);

    mg_set_request_handler(ctx, "/health", health_handler, NULL);
    register_market_routes(ctx);

    printf("stockc listening on http://localhost:%d\n", port);

    while (1) {
        sleep_ms(1000);
    }

    mg_stop(ctx);
    return 0;
}
