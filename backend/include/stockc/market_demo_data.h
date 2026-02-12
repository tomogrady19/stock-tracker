#pragma once

/**
 * Development fallback market history JSON.
 *
 * Shape:
 * {
 *   "symbol": "Demo Data",
 *   "series": [ ... reverse-chronological ... ]
 * }
 */
const char *market_demo_history_json(void);
