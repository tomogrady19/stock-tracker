#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "market_service.h"
#include "stockc/market.h"
#include "stockc/market_metrics.h"
#include "stockc/market_history_json.h"
#include "stockc/market_demo_data.h"
#include "../cache/history_cache.h"

#include "yyjson.h"

// ============================================================
// Internal helpers
// ============================================================

static int extract_latest_quote(const char *history_json,
                                struct stock_quote *out)
{
    const char *p = strstr(history_json, "\"price\":");
    if (!p) return -1;

    double latest = atof(p + 8);

    const char *p2 = strstr(p + 8, "\"price\":");
    if (!p2) return -1;

    double previous = atof(p2 + 8);

    out->price = latest;
    out->change = latest - previous;
    out->change_percent =
        (out->change / previous) * 100.0;

    return 0;
}


// ============================================================
// Service API
// ============================================================

struct market_history_result
market_service_get_history(const char *symbol, int days)
{
    struct market_history_result result;
    result.json = NULL;
    result.source = MARKET_SOURCE_DEMO;
    result.fetched_at = 0;

    const char *raw = NULL;

    // 1) Cache hit
    const char *cached = history_cache_get(symbol);
    if (cached) {
        raw = cached;
        result.source = MARKET_SOURCE_CACHE;
        result.fetched_at = history_cache_get_fetched_at(symbol);
    } else {
        // 2) Try live fetch
        char json[65536]; // large enough buffer for 252 trading days + meta data
        int rc = alpha_vantage_get_daily_history_json(
            symbol, json, sizeof(json)
        );

        if (rc == 0) {
            history_cache_set(symbol, json);
            raw = history_cache_get(symbol);
            result.source = MARKET_SOURCE_LIVE;
            result.fetched_at = history_cache_get_fetched_at(symbol);
        }
    }

    // 3) Stale cache fallback
    if (!raw) {
        const char *stale = history_cache_get(symbol);
        if (stale) {
            raw = stale;
            result.source = MARKET_SOURCE_CACHE;
            result.fetched_at = history_cache_get_fetched_at(symbol);
        }
    }

    // 4) Dev fallback
    if (!raw) {
        raw = market_demo_history_json();
        result.source = MARKET_SOURCE_DEMO;
        result.fetched_at = time(NULL);
    }

    result.json = market_build_history_with_metrics(raw, days);
    return result;
}


int market_service_get_quote(const char *symbol,
                             struct stock_quote *out)
{
    if (!out)
        return -1;

    struct market_history_result res =
        market_service_get_history(symbol, 0); /* days currently unused, but will be used soon */

    if (!res.json)
        return -1;

    memset(out, 0, sizeof(*out));
    strncpy(out->symbol, symbol, sizeof(out->symbol) - 1);

    if (extract_latest_quote(res.json, out) != 0)
        return -1;

    return 0;
}
