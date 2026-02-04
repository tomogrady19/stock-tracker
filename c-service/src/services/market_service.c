#include <string.h>
#include <stdlib.h>

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

const char *market_service_get_history(const char *symbol)
{
    const char *cached = history_cache_get(symbol);
    if (cached)
        return cached;

    char json[8192];

    int rc = alpha_vantage_get_daily_history_json(
        symbol,
        json,
        sizeof(json)
    );

    if (rc == 0) {
        history_cache_set(symbol, json);
        return history_cache_get(symbol);
    }

    const char *stale = history_cache_get(symbol);
    if (stale)
        return stale;

    return dev_fallback_history;
}

int market_service_get_quote(const char *symbol,
                             struct stock_quote *out)
{
    if (!out)
        return -1;

    const char *history = market_service_get_history(symbol);
    if (!history)
        return -1;

    memset(out, 0, sizeof(*out));
    strncpy(out->symbol, symbol, sizeof(out->symbol) - 1);

    if (extract_latest_quote(history, out) != 0)
        return -1;

    return 0;
}
