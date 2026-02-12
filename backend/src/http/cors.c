#include <string.h>
#include "cors.h"

void add_cors_headers(struct mg_connection *conn)
{
    mg_printf(conn,
        "Access-Control-Allow-Origin: http://localhost:5173\r\n"
        "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
    );
}

int handle_options_preflight(struct mg_connection *conn,
                             const struct mg_request_info *req)
{
    if (strcmp(req->request_method, "OPTIONS") != 0)
        return 0;

    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 0\r\n"
    );
    add_cors_headers(conn);
    mg_printf(conn, "\r\n");
    return 1;
}
