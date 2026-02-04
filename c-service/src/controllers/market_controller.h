#ifndef STOCKC_MARKET_CONTROLLER_H
#define STOCKC_MARKET_CONTROLLER_H

#include "civetweb.h"

/*
 * Controller functions for market endpoints.
 * These perform request → response logic, but do not
 * register routes or know about URL paths.
 */

int market_quote_controller(struct mg_connection *conn,
                            const char *symbol);

int market_history_controller(struct mg_connection *conn,
                              const char *symbol);

#endif
