#include "stockc/market_history_json.h"

#include <stdlib.h>
#include <string.h>

#include "yyjson.h"

char *
market_build_history_with_metrics(const char *history_json)
{
    if (!history_json)
        return NULL;

    yyjson_doc *doc = yyjson_read(history_json, strlen(history_json), 0);
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

    /* series is reverse-chronological → convert to chronological */
    for (size_t i = 0; i < count; i++) {
        yyjson_val *item = yyjson_arr_get(series, count - 1 - i);
        yyjson_val *price = yyjson_obj_get(item, "price");
        prices[i] = yyjson_get_real(price);
    }

    struct market_metrics metrics;
    if (market_calculate_metrics(prices, count, &metrics) != 0) {
        free(prices);
        yyjson_doc_free(doc);
        return NULL;
    }

    free(prices);

    /* Build new JSON */
    yyjson_mut_doc *mut = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *mut_root = yyjson_mut_obj(mut);
    yyjson_mut_doc_set_root(mut, mut_root);

    if (symbol && yyjson_is_str(symbol)) {
        yyjson_mut_obj_add_str(
            mut,
            mut_root,
            "symbol",
            yyjson_get_str(symbol)
        );
    }

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

    yyjson_mut_val *metrics_obj =
        yyjson_mut_obj_add_obj(mut, mut_root, "metrics");

    yyjson_mut_obj_add_real(mut, metrics_obj, "sharpe", metrics.sharpe);
    yyjson_mut_obj_add_real(mut, metrics_obj, "sortino", metrics.sortino);
    yyjson_mut_obj_add_real(
        mut, metrics_obj, "maxDrawdown", metrics.max_drawdown);
    yyjson_mut_obj_add_real(mut, metrics_obj, "cagr", metrics.cagr);

    char *out = yyjson_mut_write(mut, 0, NULL);

    yyjson_mut_doc_free(mut);
    yyjson_doc_free(doc);

    return out;
}