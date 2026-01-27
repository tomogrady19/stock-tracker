import { Router } from "express";
import { getStockHistory } from "../controllers/stocksController.js";

const router = Router();

router.get("/history", getStockHistory);

export default router;
