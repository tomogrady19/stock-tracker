#ifndef STOCKC_HTTP_RESPONSES_H
#define STOCKC_HTTP_RESPONSES_H

#include "civetweb.h"

/*
 * Common HTTP JSON responses
 */

//TODO make variable names consistent 
void send_json_error(struct mg_connection *conn, int status_code, const char *message);

void send_json_response(struct mg_connection *conn, int status, const char *json);

#endif