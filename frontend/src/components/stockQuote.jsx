import { useEffect, useState } from "react";
import { fetchStockQuote, fetchStockHistory } from "../services/stockApi";
import StockChart from "./stockChart";

export default function StockQuote({ symbol }) {
  const [quote, setQuote] = useState(null);
  const [history, setHistory] = useState(null);
  const [error, setError] = useState(null);

  useEffect(() => {
    // reset state when symbol changes
    setQuote(null);
    setHistory(null);
    setError(null);

    Promise.all([fetchStockQuote(symbol), fetchStockHistory(symbol)])
      .then(([quoteData, historyData]) => {
        setQuote(quoteData);
        setHistory(historyData);
      })
      .catch((err) => setError(err.message));
  }, [symbol]);

  if (error) {
    return <p style={{ color: "red" }}>Error: {error}</p>;
  }

  if (!quote || !history) {
    return <p>Loading...</p>;
  }

  const source = history.source;
  const fetchedAt = history.fetchedAt;

  let freshnessText = null;

  if (source === "cache" && fetchedAt) {
    const ageSeconds = Math.floor(Date.now() / 1000 - fetchedAt);
    const ageMinutes = Math.floor(ageSeconds / 60);
    const ageHours = Math.floor(ageMinutes / 60);

    if (ageMinutes < 1) {
      freshnessText = "Last updated just now";
    } else if (ageMinutes < 60) {
      freshnessText = `Last updated ${ageMinutes} minute${ageMinutes === 1 ? "" : "s"} ago`;
    } else if (ageHours < 24) {
      freshnessText = `Last updated ${ageHours} hour${ageHours === 1 ? "" : "s"} ago`;
    } else {
      const ageDays = Math.floor(ageHours / 24);
      freshnessText = `Last updated ${ageDays} day${ageDays === 1 ? "" : "s"} ago`;
    }
  }

  return (
    <div className="stock-card" style={{ display: "flex", gap: "20px" }}>

      <div style={{ flex: 2 }}>
        <StockChart series={history.series} />
      </div>

      {/* //TODO move the styles below to a css file */}
      <div style={{ flex: 1 }}>
        <h2>{quote.symbol}</h2>
        {source === "demo" && (
          <div
            style={{
              backgroundColor: "#fff3cd",
              color: "#856404",
              padding: "10px",
              marginBottom: "10px",
              borderRadius: "6px",
              fontSize: "0.9rem",
            }}
          >
            ⚠ Live market data unavailable — showing demo data
          </div>
        )}

        {source === "cache" && freshnessText && (
          <div
            style={{
              color: "#666",
              fontSize: "0.85rem",
              marginBottom: "10px",
            }}
          >
            {freshnessText}
          </div>
        )}

        <p>Price: ${quote.price.toFixed(2)}</p>
        <p>
          Change: {quote.change.toFixed(2)} ({quote.changePercent.toFixed(2)}%)
        </p>
      </div>
    </div>
  );
}
