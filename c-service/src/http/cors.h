#ifndef STOCKC_HTTP_CORS_H
#define STOCKC_HTTP_CORS_H

#include "civetweb.h"

/*
 * CORS utilities
 */

void add_cors_headers(struct mg_connection *conn);

/*
 * Handles OPTIONS preflight requests.
 * Returns 1 if handled, 0 otherwise.
 */
int handle_options_preflight(struct mg_connection *conn,
                             const struct mg_request_info *req);

#endif