import { useState } from "react";
import StockQuote from "./components/stockQuote";
import "./App.css";

export default function App() {
  const [inputSymbol, setInputSymbol] = useState("AAPL");
  const [activeSymbol, setActiveSymbol] = useState("AAPL");
  const [days, setDays] = useState(30);

  function handleSubmit(e) {
    e.preventDefault();

    const trimmed = inputSymbol.trim().toUpperCase();
    if (trimmed.length === 0) return;

    setActiveSymbol(trimmed);
  }

  return (
    <div className="app" style={{ minHeight: "100vh", display: "flex", flexDirection: "column" }}>
      <div style={{ flex: 1 }}>
        <h1>Stock Tracker</h1>

        <form onSubmit={handleSubmit} style={{ marginBottom: "1rem" }}>
          <input
            type="text"
            value={inputSymbol}
            onChange={(e) => setInputSymbol(e.target.value)}
            placeholder="Enter symbol (e.g. AAPL)"
            style={{ marginRight: "0.5rem" }}
          />

          <select
            value={days}
            onChange={(e) => setDays(Number(e.target.value))}
            style={{ marginRight: "0.5rem" }}
          >
            <option value={5}>5D</option>
            <option value={30}>1M</option>
            <option value={90}>3M</option>
            <option value={100}>Full</option>
          </select>

          <button type="submit">Load</button>
        </form>

        <StockQuote symbol={activeSymbol} days={days} />
      </div>

      {/* Footer */}
      <footer
        style={{
          textAlign: "center",
          padding: "20px 0",
          fontSize: "0.9rem",
          color: "#666",
        }}
      >
        Made by{" "}
        <a
          href="https://github.com/tomogrady19"
          target="_blank"
          rel="noopener noreferrer"
          style={{ color: "#0077cc", textDecoration: "none", fontWeight: "500" }}
        >
          Tom O'Grady
        </a>
      </footer>
    </div>
  );
}