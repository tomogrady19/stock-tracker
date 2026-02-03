#ifndef STOCKC_HISTORY_CACHE_H
#define STOCKC_HISTORY_CACHE_H

#include <stddef.h>
#include <time.h>

/*
 * History cache entry.
 * Stores JSON already formatted for the API response.
 */
struct history_cache_entry {
    char symbol[16];
    time_t fetched_at;
    char json[8192];
};

/*
 * Initialize the cache (currently a no-op, but future-proof).
 */
void history_cache_init(void);

/*
 * Check whether cached data for `symbol` is still valid.
 * Returns 1 if valid, 0 otherwise.
 */
int history_cache_is_valid(const char *symbol);

/*
 * Get cached JSON for `symbol`.
 * Returns pointer to internal buffer, or NULL if invalid.
 */
const char *history_cache_get(const char *symbol);

/*
 * Store JSON in cache for `symbol` with current timestamp.
 */
void history_cache_set(const char *symbol, const char *json);

#endif /* STOCKC_HISTORY_CACHE_H */
