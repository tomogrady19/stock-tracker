#include <string.h>

#include "civetweb.h"
#include "../controllers/market_controller.h"
#include "../http/cors.h"
#include "../http/responses.h"


// ============================================================
// Helpers (route-specific)
// ============================================================

static int extract_symbol_param(const struct mg_request_info *req,
                                char *out,
                                size_t out_size)
{
    if (!req->query_string)
        return 0;

    mg_get_var(req->query_string,
               strlen(req->query_string),
               "symbol",
               out,
               out_size);

    return strlen(out) > 0;
}


// ============================================================
// Route handlers (HTTP glue only)
// ============================================================

static int handle_market_quote(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *req = mg_get_request_info(conn);

    if (handle_options_preflight(conn, req))
        return 1;

    char symbol[16] = {0};
    if (!extract_symbol_param(req, symbol, sizeof(symbol))) {
        send_json_error(conn, 400, "symbol parameter required");
        return 1;
    }

    return market_quote_controller(conn, symbol);
}

static int handle_market_history(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *req = mg_get_request_info(conn);

    if (handle_options_preflight(conn, req))
        return 1;

    char symbol[16] = {0};
    if (!extract_symbol_param(req, symbol, sizeof(symbol))) {
        send_json_error(conn, 400, "symbol parameter required");
        return 1;
    }

    return market_history_controller(conn, symbol);
}


// ============================================================
// Route registration
// ============================================================

void register_market_routes(struct mg_context *ctx)
{
    mg_set_request_handler(ctx,
        "/api/market/quote",
        handle_market_quote,
        NULL);

    mg_set_request_handler(ctx,
        "/api/market/history",
        handle_market_history,
        NULL);
}
