#include "history_cache.h"

#include <string.h>
#include <time.h>

#define HISTORY_CACHE_TTL 60  // seconds

/*
 * Single-entry cache for now.
 * This mirrors the existing behavior exactly.
 */
static struct history_cache_entry cache = {0};

void history_cache_init(void)
{
    // No-op for now
}

int history_cache_is_valid(const char *symbol)
{
    if (!symbol)
        return 0;

    if (strcmp(cache.symbol, symbol) != 0)
        return 0;

    time_t now = time(NULL);
    return difftime(now, cache.fetched_at) < HISTORY_CACHE_TTL;
}

const char *history_cache_get(const char *symbol)
{
    if (!history_cache_is_valid(symbol))
        return NULL;

    return cache.json;
}

void history_cache_set(const char *symbol, const char *json)
{
    if (!symbol || !json)
        return;

    strncpy(cache.symbol, symbol, sizeof(cache.symbol) - 1);
    cache.symbol[sizeof(cache.symbol) - 1] = '\0';

    strncpy(cache.json, json, sizeof(cache.json) - 1);
    cache.json[sizeof(cache.json) - 1] = '\0';

    cache.fetched_at = time(NULL);
}
