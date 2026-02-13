#include <string.h>
#include <stdlib.h>
#include "cors.h"

/*
 * Returns the allowed origin.
 * Production: from CORS_ALLOWED_ORIGIN env var
 * Local dev fallback: http://localhost:5173
 */
static const char *get_allowed_origin(void)
{
    const char *env_origin = getenv("CORS_ALLOWED_ORIGIN");

    if (env_origin && strlen(env_origin) > 0)
        return env_origin;

    // Local development fallback
    return "http://localhost:5173";
}

void add_cors_headers(struct mg_connection *conn)
{
    const char *origin = get_allowed_origin();

    mg_printf(conn,
        "Access-Control-Allow-Origin: %s\r\n"
        "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n",
        origin
    );
}

int handle_options_preflight(struct mg_connection *conn,
                             const struct mg_request_info *req)
{
    if (strcmp(req->request_method, "OPTIONS") != 0)
        return 0;

    const char *origin = get_allowed_origin();

    mg_printf(conn,
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: %s\r\n"
        "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: 0\r\n"
        "\r\n",
        origin
    );

    return 1;
}