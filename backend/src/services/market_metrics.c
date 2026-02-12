#include "stockc/market_metrics.h"

#include <math.h>

int market_calculate_metrics(
    const double *prices,
    size_t count,
    struct market_metrics *out
)
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