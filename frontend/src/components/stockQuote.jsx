import { useEffect, useRef, useState } from "react";
import { fetchStockQuote } from "../services/stockApi";

export default function StockQuote({ symbol }) {
    const [quote, setQuote] = useState(null);
    const [error, setError] = useState(null);
    const hasFetched = useRef(false);

    useEffect(() => {
        if (hasFetched.current) return;
        hasFetched.current = true;

        fetchStockQuote(symbol)
            .then(setQuote)
            .catch((err) => setError(err.message));
    }, [symbol]);

    if (error) {
        return <p style={{ color: "red" }}>Error: {error}</p>;
    }

    if (!quote) {
        return <p>Loading...</p>;
    }

    return (
        <div className="stock-card">
            <h2>{quote.symbol}</h2>
            <p>Price: ${quote.price.toFixed(2)}</p>
            <p>
            Change: {quote.change.toFixed(2)} (
            {quote.changePercent.toFixed(2)}%)
            </p>
        </div>
        );
}
