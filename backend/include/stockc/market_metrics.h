#pragma once

#include <stddef.h>

#define TRADING_DAYS_PER_YEAR 252.0

struct market_metrics {
    double sharpe;
    double sortino;
    double max_drawdown;
    double cagr;
};

/**
 * Calculate risk/return metrics from a price series.
 *
 * prices must be in chronological order (oldest -> newest).
 * count must be >= 2.
 *
 * Returns 0 on success, -1 on failure.
 */
int market_calculate_metrics(
    const double *prices,
    size_t count,
    struct market_metrics *out
);