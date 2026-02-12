#pragma once

#include <stddef.h>

#include "stockc/market_metrics.h"

/**
 * Build a history JSON string with metrics injected.
 *
 * - history_json: raw history JSON (symbol + series)
 * - returns a newly allocated JSON string (caller must free)
 *
 * Returns NULL on failure.
 */
char *market_build_history_with_metrics(
    const char *history_json,
    int days
);
