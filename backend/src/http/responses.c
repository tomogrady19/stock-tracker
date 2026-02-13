#include <stdio.h>
#include <string.h>
#include "responses.h"
#include "cors.h"

static const char *status_text(int status_code)
{
    switch (status_code) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default:  return "OK";
    }
}

void send_json_response(struct mg_connection *conn,
                        int status_code,
                        const char *json_body)
{
    size_t body_len = strlen(json_body);

    mg_printf(conn,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n",
        status_code,
        status_text(status_code),
        body_len
    );

    add_cors_headers(conn);

    mg_printf(conn, "\r\n");
    mg_write(conn, json_body, body_len);
}

void send_json_error(struct mg_connection *conn,
                     int status_code,
                     const char *message)
{
    char buffer[512];

    snprintf(buffer, sizeof(buffer),
             "{\"error\":\"%s\"}", message);

    send_json_response(conn, status_code, buffer);
}