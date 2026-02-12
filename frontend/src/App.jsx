import { useState } from "react";
import StockQuote from "./components/StockQuote";
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
    <div className="app">
      <h1>Stock Tracker</h1>

      <form onSubmit={handleSubmit} style={{ marginBottom: "1rem" }}>
        <input
          type="text"
          value={inputSymbol}
          onChange={(e) => setInputSymbol(e.target.value)}
          placeholder="Enter symbol (e.g. AAPL)"
          style={{ marginRight: "0.5rem" }}
        />

        {/* NEW window selector */}
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
  );
}