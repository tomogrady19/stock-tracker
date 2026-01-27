import { getDailyHistory } from "../services/alphaVantageService.js";

export async function getStockHistory(req, res) {
    try {
        const { symbol } = req.query;

        if (!symbol) {
            return res.status(400).json({ error: "Missing stock symbol" });
        }

        const data = await getDailyHistory(symbol);
        res.json(data);
    } catch (err) {
        console.error(err);
        res.status(500).json({ error: "Failed to fetch stock data" });
    }
}