#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "market_service.h"
#include "stockc/market.h"
#include "../cache/history_cache.h"

#include "yyjson.h"


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
// Metrics
// ============================================================

#define TRADING_DAYS_PER_YEAR 252.0

struct market_metrics {
    double sharpe;
    double sortino;
    double max_drawdown;
    double cagr;
};

static int
calculate_metrics(const double *prices,
                  size_t count,
                  struct market_metrics *out)
{
    if (!prices || !out || count < 2)
        return -1;

    double returns_sum = 0.0;
    double returns_sq_sum = 0.0;

    double downside_sq_sum = 0.0;
    size_t downside_count = 0;

    double peak = prices[0];
    double max_drawdown = 0.0;

    for (size_t i = 1; i < count; i++) {
        double r = (prices[i] / prices[i - 1]) - 1.0;

        returns_sum += r;
        returns_sq_sum += r * r;

        if (r < 0.0) {
            downside_sq_sum += r * r;
            downside_count++;
        }

        if (prices[i] > peak)
            peak = prices[i];

        double drawdown = (prices[i] - peak) / peak;
        if (drawdown < max_drawdown)
            max_drawdown = drawdown;
    }

    double mean = returns_sum / (count - 1);
    double variance =
        (returns_sq_sum / (count - 1)) - (mean * mean);

    double stddev = variance > 0.0 ? sqrt(variance) : 0.0;

    double downside_stddev = 0.0;
    if (downside_count > 0) {
        downside_stddev =
            sqrt(downside_sq_sum / downside_count);
    }

    out->sharpe =
        stddev > 0.0
            ? (mean / stddev) * sqrt(TRADING_DAYS_PER_YEAR)
            : 0.0;

    out->sortino =
        downside_stddev > 0.0
            ? (mean / downside_stddev) * sqrt(TRADING_DAYS_PER_YEAR)
            : 0.0;

    out->max_drawdown = max_drawdown;

    double years = (double)count / TRADING_DAYS_PER_YEAR;
    out->cagr =
        years > 0.0
            ? pow(prices[count - 1] / prices[0], 1.0 / years) - 1.0
            : 0.0;

    return 0;
}


// ============================================================
// JSON augmentation
// ============================================================

static char *
build_history_with_metrics(const char *history_json)
{
    yyjson_doc *doc = yyjson_read(
        history_json,
        strlen(history_json),
        0
    );
    if (!doc)
        return NULL;

    yyjson_val *root = yyjson_doc_get_root(doc);
    yyjson_val *series = yyjson_obj_get(root, "series");
    yyjson_val *symbol = yyjson_obj_get(root, "symbol");

    if (!series || !yyjson_is_arr(series)) {
        yyjson_doc_free(doc);
        return NULL;
    }

    size_t count = yyjson_arr_size(series);
    if (count < 2) {
        yyjson_doc_free(doc);
        return strdup(history_json);
    }

    double *prices = malloc(sizeof(double) * count);
    if (!prices) {
        yyjson_doc_free(doc);
        return NULL;
    }

    // series is reverse-chronological → read backwards
    for (size_t i = 0; i < count; i++) {
        yyjson_val *item =
            yyjson_arr_get(series, count - 1 - i);
        yyjson_val *price = yyjson_obj_get(item, "price");
        prices[i] = yyjson_get_real(price);
    }

    struct market_metrics metrics;
    if (calculate_metrics(prices, count, &metrics) != 0) {
        free(prices);
        yyjson_doc_free(doc);
        return NULL;
    }

    free(prices);

    // ---------- build new JSON ----------
    yyjson_mut_doc *mut = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *mut_root = yyjson_mut_obj(mut);

    yyjson_mut_doc_set_root(mut, mut_root);

    // symbol
    if (symbol && yyjson_is_str(symbol)) {
        yyjson_mut_obj_add_str(
            mut,
            mut_root,
            "symbol",
            yyjson_get_str(symbol)
        );
    }

    // series (rebuild, preserving order)
    yyjson_mut_val *mut_series =
        yyjson_mut_obj_add_arr(mut, mut_root, "series");

    for (size_t i = 0; i < yyjson_arr_size(series); i++) {
        yyjson_val *item = yyjson_arr_get(series, i);

        yyjson_val *date = yyjson_obj_get(item, "date");
        yyjson_val *price = yyjson_obj_get(item, "price");

        yyjson_mut_val *mut_item =
            yyjson_mut_arr_add_obj(mut, mut_series);

        if (date && yyjson_is_str(date)) {
            yyjson_mut_obj_add_str(
                mut,
                mut_item,
                "date",
                yyjson_get_str(date)
            );
        }

        yyjson_mut_obj_add_real(
            mut,
            mut_item,
            "price",
            yyjson_get_real(price)
        );
    }

    // metrics
    yyjson_mut_val *metrics_obj =
        yyjson_mut_obj_add_obj(mut, mut_root, "metrics");

    yyjson_mut_obj_add_real(mut, metrics_obj, "sharpe", metrics.sharpe);
    yyjson_mut_obj_add_real(mut, metrics_obj, "sortino", metrics.sortino);
    yyjson_mut_obj_add_real(mut, metrics_obj, "maxDrawdown", metrics.max_drawdown);
    yyjson_mut_obj_add_real(mut, metrics_obj, "cagr", metrics.cagr);

    char *out = yyjson_mut_write(mut, 0, NULL);

    yyjson_mut_doc_free(mut);
    yyjson_doc_free(doc);

    return out;
}


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

    const char *raw = NULL;

    // 1) Cache hit
    const char *cached = history_cache_get(symbol);
    if (cached) {
        raw = cached;
        result.source = MARKET_SOURCE_CACHE;
        result.fetched_at = history_cache_get_fetched_at(symbol);
    } else {
        // 2) Try live fetch
        char json[8192];
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
        raw = dev_fallback_history;
        result.source = MARKET_SOURCE_DEMO;
        result.fetched_at = time(NULL);
    }

    result.json = build_history_with_metrics(raw);
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
