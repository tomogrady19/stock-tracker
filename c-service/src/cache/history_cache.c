#include "history_cache.h"

#include <string.h>
#include <time.h>

#define HISTORY_CACHE_MAX_ENTRIES 16
#define HISTORY_CACHE_TTL_SECONDS (60 * 60 * 24)  // 24 hours (since I'm fetching daily data)

/*
 * Fixed-size, in-memory cache for historical stock data.
 *
 * Policy:
 * - Keyed by exact symbol match
 * - Absolute TTL per entry
 * - Up to HISTORY_CACHE_MAX_ENTRIES entries
 * - On insert:
 *     - Reuse an expired slot if available
 *     - Otherwise evict the oldest (earliest fetched_at) entry
 */

static struct history_cache_entry cache[HISTORY_CACHE_MAX_ENTRIES] = {0};

void history_cache_init(void)
{
    // No-op for now
}

static int entry_is_valid(const struct history_cache_entry *entry, const char *symbol)
{
    if (!entry || !symbol)
        return 0;

    if (entry->symbol[0] == '\0')
        return 0;

    if (strcmp(entry->symbol, symbol) != 0)
        return 0;

    time_t now = time(NULL);
    return difftime(now, entry->fetched_at) < HISTORY_CACHE_TTL_SECONDS;
}

int history_cache_is_valid(const char *symbol)
{
    for (int i = 0; i < HISTORY_CACHE_MAX_ENTRIES; i++)
    {
        if (entry_is_valid(&cache[i], symbol))
            return 1;
    }

    return 0;
}

const char *history_cache_get(const char *symbol)
{
    for (int i = 0; i < HISTORY_CACHE_MAX_ENTRIES; i++)
    {
        if (entry_is_valid(&cache[i], symbol))
            return cache[i].json;
    }

    return NULL;
}

void history_cache_set(const char *symbol, const char *json)
{
    if (!symbol || !json)
        return;

    time_t now = time(NULL);

    /* 1) Prefer an expired or empty slot */
    for (int i = 0; i < HISTORY_CACHE_MAX_ENTRIES; i++)
    {
        if (cache[i].symbol[0] == '\0' ||
            difftime(now, cache[i].fetched_at) >= HISTORY_CACHE_TTL_SECONDS)
        {
            strncpy(cache[i].symbol, symbol, sizeof(cache[i].symbol) - 1);
            cache[i].symbol[sizeof(cache[i].symbol) - 1] = '\0';

            strncpy(cache[i].json, json, sizeof(cache[i].json) - 1);
            cache[i].json[sizeof(cache[i].json) - 1] = '\0';

            cache[i].fetched_at = now;
            return;
        }
    }

    /* 2) Evict the oldest entry */
    int oldest_index = 0;
    time_t oldest_time = cache[0].fetched_at;

    for (int i = 1; i < HISTORY_CACHE_MAX_ENTRIES; i++)
    {
        if (cache[i].fetched_at < oldest_time)
        {
            oldest_time = cache[i].fetched_at;
            oldest_index = i;
        }
    }

    strncpy(cache[oldest_index].symbol, symbol, sizeof(cache[oldest_index].symbol) - 1);
    cache[oldest_index].symbol[sizeof(cache[oldest_index].symbol) - 1] = '\0';

    strncpy(cache[oldest_index].json, json, sizeof(cache[oldest_index].json) - 1);
    cache[oldest_index].json[sizeof(cache[oldest_index].json) - 1] = '\0';

    cache[oldest_index].fetched_at = now;
}
