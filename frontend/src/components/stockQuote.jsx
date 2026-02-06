import { useEffect, useState } from "react";
import { fetchStockHistory } from "../services/stockApi";
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

    fetchStockHistory(symbol)
      .then((historyData) => {
        setHistory(historyData);

        const series = historyData.series;
        if (series && series.length >= 2) {
          // series is reverse-chronological (latest first)
          const latest = series[0].price;
          const previous = series[1].price;

          const change = latest - previous;
          const changePercent = (change / previous) * 100;

          setQuote({
            symbol: historyData.symbol ?? symbol,
            price: latest,
            change,
            changePercent,
          });
        }
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
  const metrics = history.metrics ?? null;

  let freshnessText = null;

  if (source === "cache" && fetchedAt) {
    const ageSeconds = Math.floor(Date.now() / 1000 - fetchedAt);
    const ageMinutes = Math.floor(ageSeconds / 60);
    const ageHours = Math.floor(ageMinutes / 60);

    if (ageMinutes < 1) {
      freshnessText = "As of just now";
    } else if (ageMinutes < 60) {
      freshnessText = `As of ${ageMinutes} minute${ageMinutes === 1 ? "" : "s"} ago`;
    } else if (ageHours < 24) {
      freshnessText = `As of ${ageHours} hour${ageHours === 1 ? "" : "s"} ago`;
    } else {
      const ageDays = Math.floor(ageHours / 24);
      freshnessText = `As of ${ageDays} day${ageDays === 1 ? "" : "s"} ago`;
    }
  }

  return (
    <div className="stock-card" style={{ display: "flex", gap: "20px" }}>
      <div style={{ flex: 2 }}>
        <StockChart
          series={history.series}
          symbol={quote.symbol}
        />
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
            ⚠ Live market data unavailable. Showing demo data.
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

        {/* ================= Metrics ================= */}
        {metrics && (
          <div style={{ marginTop: "14px", fontSize: "0.95rem" }}>
            <div title="Risk-adjusted return (rf = 0%)">
              Sharpe:{" "}
              {metrics.sharpe !== 0
                ? metrics.sharpe.toFixed(2)
                : "—"}
            </div>

            <div title="Downside-risk-adjusted return (rf = 0%)">
              Sortino:{" "}
              {metrics.sortino !== 0
                ? metrics.sortino.toFixed(2)
                : "—"}
            </div>

            <div title="Worst peak-to-trough loss">
              Max Drawdown:{" "}
              {(metrics.maxDrawdown * 100).toFixed(1)}%
            </div>

            <div title="Compound annual growth rate over this period">
              CAGR:{" "}
              {(metrics.cagr * 100).toFixed(1)}%
            </div>
          </div>
        )}
      </div>
    </div>
  );
}