#ifndef STOCKC_HTTP_RESPONSES_H
#define STOCKC_HTTP_RESPONSES_H

#include "civetweb.h"

/*
 * Common HTTP JSON responses
 */

void send_json_error(struct mg_connection *conn,
                     int status_code,
                     const char *message);

#endif