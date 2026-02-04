#ifndef STOCKC_MARKET_SERVICE_H
#define STOCKC_MARKET_SERVICE_H

/*
 * Market service layer.
 * Encapsulates cache usage, Alpha Vantage calls,
 * and fallback behaviour.
 */

/*
 * Returns a pointer to a JSON history payload.
 * The returned pointer is valid for the lifetime
 * of the request.
 */
const char *market_service_get_history(const char *symbol);

/*
 * Derives a stock quote from history data.
 * Returns 0 on success, non-zero on failure.
 */
int market_service_get_quote(const char *symbol,
                             struct stock_quote *out);

#endif
