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
  "\"symbol\":\"Demo Data\","
  "\"series\":["
    "{\"date\":\"2026-02-04\",\"price\":276.49},"
    "{\"date\":\"2026-02-03\",\"price\":269.48},"
    "{\"date\":\"2026-02-02\",\"price\":270.01},"
    "{\"date\":\"2026-01-30\",\"price\":259.48},"
    "{\"date\":\"2026-01-29\",\"price\":258.28},"
    "{\"date\":\"2026-01-28\",\"price\":256.44},"
    "{\"date\":\"2026-01-27\",\"price\":258.27},"
    "{\"date\":\"2026-01-26\",\"price\":255.41},"
    "{\"date\":\"2026-01-23\",\"price\":248.04},"
    "{\"date\":\"2026-01-22\",\"price\":248.35},"
    "{\"date\":\"2026-01-21\",\"price\":247.65},"
    "{\"date\":\"2026-01-20\",\"price\":246.70},"
    "{\"date\":\"2026-01-16\",\"price\":255.53},"
    "{\"date\":\"2026-01-15\",\"price\":258.21},"
    "{\"date\":\"2026-01-14\",\"price\":259.96},"
    "{\"date\":\"2026-01-13\",\"price\":261.05},"
    "{\"date\":\"2026-01-12\",\"price\":260.25},"
    "{\"date\":\"2026-01-09\",\"price\":259.37},"
    "{\"date\":\"2026-01-08\",\"price\":259.04},"
    "{\"date\":\"2026-01-07\",\"price\":260.33},"
    "{\"date\":\"2026-01-06\",\"price\":262.36},"
    "{\"date\":\"2026-01-05\",\"price\":267.26},"
    "{\"date\":\"2026-01-02\",\"price\":271.01},"
    "{\"date\":\"2025-12-31\",\"price\":271.86},"
    "{\"date\":\"2025-12-30\",\"price\":273.08},"
    "{\"date\":\"2025-12-29\",\"price\":273.76},"
    "{\"date\":\"2025-12-26\",\"price\":273.40},"
    "{\"date\":\"2025-12-24\",\"price\":273.81},"
    "{\"date\":\"2025-12-23\",\"price\":272.36},"
    "{\"date\":\"2025-12-22\",\"price\":270.97}"
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

    // 1) Cache hit
    const char *cached = history_cache_get(symbol);
    if (cached) {
        result.json = cached;
        result.source = MARKET_SOURCE_CACHE;
        result.fetched_at = history_cache_get_fetched_at(symbol);
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
        result.fetched_at = history_cache_get_fetched_at(symbol);
        return result;
    }

    // 3) Stale cache fallback
    const char *stale = history_cache_get(symbol);
    if (stale) {
        result.json = stale;
        result.source = MARKET_SOURCE_CACHE;
        result.fetched_at = history_cache_get_fetched_at(symbol);
        return result;
    }

    // 4) Dev fallback
    result.json = dev_fallback_history;
    result.source = MARKET_SOURCE_DEMO;
    result.fetched_at = time(NULL);

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
