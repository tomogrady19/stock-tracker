#ifndef STOCKC_MARKET_SERVICE_H
#define STOCKC_MARKET_SERVICE_H

#include <time.h>
#include "stockc/market.h"

/*
 * Where the history data came from
 */
enum market_data_source {
    MARKET_SOURCE_LIVE,
    MARKET_SOURCE_CACHE,
    MARKET_SOURCE_DEMO
};

/*
 * Result object returned by the history service
 */
struct market_history_result {
    const char *json;           // history JSON payload
    enum market_data_source source;
    time_t fetched_at;          // when the data was originally fetched
};

/*
 * Returns history + metadata
 */
struct market_history_result
market_service_get_history(const char *symbol, int days);

/*
 * Quote logic stays the same externally
 */
int market_service_get_quote(const char *symbol,
                             struct stock_quote *out);

#endif
