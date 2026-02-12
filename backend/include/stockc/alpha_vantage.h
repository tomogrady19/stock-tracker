#pragma once

#include "stockc/market.h"

#ifdef __cplusplus
extern "C" {
#endif

// Fetch live market data from Alpha Vantage.
// Returns 0 on success, non-zero on failure.
int alpha_vantage_get_quote(const char *symbol, struct stock_quote *out);

#ifdef __cplusplus
}
#endif