import { useState } from "react";
import StockQuote from "./components/StockQuote";
import "./App.css";

export default function App() {
  const [inputSymbol, setInputSymbol] = useState("AAPL");
  const [activeSymbol, setActiveSymbol] = useState("AAPL");

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
        <button type="submit">Load</button>
      </form>

      <StockQuote symbol={activeSymbol} />
    </div>
  );
}