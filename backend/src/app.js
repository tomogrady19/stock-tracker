import express from "express";
import cors from "cors";
import "dotenv/config";
import stocksRoutes from "./routes/stocks.js";

const app = express();

app.use(cors());
app.use(express.json());

app.use("/api/stocks", stocksRoutes)

app.get("/api/health", (req, res) => {
    res.json({ status: "ok" });
});

const PORT = 4000;
app.listen(PORT, () => console.log(`API running on http://localhost:${PORT}`));

// To run the server, use the command: npm run dev