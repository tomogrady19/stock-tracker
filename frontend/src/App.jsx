import { useEffect, useRef, useState } from "react";
import "./App.css";

export default function App() {
  const [quote, setQuote] = useState(null);
  const [error, setError] = useState(null);
  const hasFetched = useRef(false);

  useEffect(() => {
    if (hasFetched.current) return;
    hasFetched.current = true;

    fetch("http://localhost:8080/api/market/quote?symbol=AAPL")
      .then((res) => {
        if (!res.ok) throw new Error("Request failed");
        return res.json();
      })
      .then((data) => {
        setQuote(data);
      })
      .catch((err) => {
        setError(err.message);
      });
  }, []);

  return (
    <div style={{ padding: "2rem", fontFamily: "system-ui, sans-serif" }}>
      <h1>Stock Tracker</h1>

      {error && <p style={{ color: "red" }}>Error: {error}</p>}

      {!quote && !error && <p>Loading...</p>}

      {quote && (
        <div
          style={{
            border: "1px solid #ccc",
            borderRadius: "8px",
            padding: "1rem",
            maxWidth: "320px",
          }}
        >
          <h2>{quote.symbol}</h2>
          <p>Price: ${quote.price.toFixed(2)}</p>
          <p>
            Change: {quote.change.toFixed(2)} (
            {quote.changePercent.toFixed(2)}%)
          </p>
        </div>
      )}
    </div>
  );
}
