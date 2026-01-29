const BASE_URL = "http://localhost:8080";

export async function fetchStockQuote(symbol) {
    const url = `${BASE_URL}/api/market/quote?symbol=${symbol}`;

    const res = await fetch(url);

    if (!res.ok) {
        throw new Error(`Request failed with status ${res.status}`);
    }

    return res.json();
}
