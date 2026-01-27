import { useEffect, useState } from "react";

function App() {
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  useEffect(() => {
    async function fetchStock() {
      try {
        const res = await fetch(
          "http://localhost:4000/api/stocks/history?symbol=AAPL"
        );

        if (!res.ok) {
          throw new Error("Failed to fetch data");
        }

        const json = await res.json();
        setData(json);
      } catch (err) {
        setError(err.message);
      } finally {
        setLoading(false);
      }
    }

    fetchStock();
  }, []);

  if (loading) return <p>Loading stock data...</p>;
  if (error) return <p>Error: {error}</p>;

  return (
    <div style={{ padding: 20 }}>
      <h1>Stock Tracker</h1>
      <pre>{JSON.stringify(data, null, 2)}</pre>
    </div>
  );
}

export default App;
