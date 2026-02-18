#pragma once

#include "stockc/market.h"

#ifdef __cplusplus
extern "C" {
#endif

// Fetch live market data from Alpha Vantage.
// Returns 0 on success, non-zero on failure.
int alpha_vantage_get_quote(const char *symbol, struct stock_quote *out);

int alpha_vantage_get_daily_history_json(const char *symbol, char **out_json);

#ifdef __cplusplus
}
#endif