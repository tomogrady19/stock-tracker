#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "market_service.h"
#include "stockc/market.h"
#include "../cache/history_cache.h"


// ============================================================
// DEV fallback history
// ============================================================

static const char *dev_fallback_history =
"{"
  "\"symbol\":\"AAPL\","
  "\"series\":["
    "{\"date\":\"2026-01-27\",\"price\":252.10},"
    "{\"date\":\"2026-01-28\",\"price\":258.27},"
    "{\"date\":\"2026-01-29\",\"price\":256.44},"
    "{\"date\":\"2026-01-30\",\"price\":260.05},"
    "{\"date\":\"2026-02-02\",\"price\":259.40}"
  "]"
"}";


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
market_service_get_history(const char *symbol)
{
    struct market_history_result result;
    result.json = NULL;
    result.source = MARKET_SOURCE_DEMO;
    result.fetched_at = 0;

    time_t now = time(NULL);

    // 1) Try cache first
    const char *cached = history_cache_get(symbol);
    if (cached) {
        result.json = cached;
        result.source = MARKET_SOURCE_CACHE;
        result.fetched_at = now;   // Approximate (we don’t expose cache timestamp yet)
        return result;
    }

    // 2) Try live fetch
    char json[8192];

    int rc = alpha_vantage_get_daily_history_json(
        symbol,
        json,
        sizeof(json)
    );

    if (rc == 0) {
        history_cache_set(symbol, json);

        result.json = history_cache_get(symbol);
        result.source = MARKET_SOURCE_LIVE;
        result.fetched_at = now;
        return result;
    }

    // 3) Try stale cache (rare path)
    const char *stale = history_cache_get(symbol);
    if (stale) {
        result.json = stale;
        result.source = MARKET_SOURCE_CACHE;
        result.fetched_at = now;
        return result;
    }

    // 4) Final fallback: demo data
    result.json = dev_fallback_history;
    result.source = MARKET_SOURCE_DEMO;
    result.fetched_at = now;

    return result;
}


int market_service_get_quote(const char *symbol,
                             struct stock_quote *out)
{
    if (!out)
        return -1;

    struct market_history_result res =
        market_service_get_history(symbol);

    if (!res.json)
        return -1;

    memset(out, 0, sizeof(*out));
    strncpy(out->symbol, symbol, sizeof(out->symbol) - 1);

    if (extract_latest_quote(res.json, out) != 0)
        return -1;

    return 0;
}
