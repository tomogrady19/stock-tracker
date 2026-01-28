#pragma once

struct stock_quote {
    char symbol[16];
    double price;
    double change;
    double change_percent;
};

int market_get_quote(const char *symbol, struct stock_quote *out);