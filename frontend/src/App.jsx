import StockQuote from "./components/stockQuote";
import "./App.css";

export default function App() {
  return (
    <div className="app">
      <h1>Stock Tracker</h1>
      <StockQuote symbol="AAPL" />
    </div>
  );
}
