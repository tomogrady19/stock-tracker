#include "responses.h"
#include "cors.h"

void send_json_error(struct mg_connection *conn,
                     int status_code,
                     const char *message)
{
    mg_printf(conn,
        "HTTP/1.1 %d Bad Request\r\n"
        "Content-Type: application/json\r\n",
        status_code
    );
    add_cors_headers(conn);
    mg_printf(conn,
        "\r\n{\"error\":\"%s\"}",
        message
    );
}
