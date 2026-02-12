# Stock Tracker

A full-stack web application for fetching, analysing, and visualising stock market data.  
The backend is implemented in C, which also computes financial metrics. React frontend then plots the data and displays the metrics.

---

## Frontend
- React (Vite)
- JavaScript
- Chart.js (via react-chartjs-2)

## Backend
- C (C11)
- CivetWeb (HTTP server)
- libcurl (HTTP client)
- yyjson (JSON parsing)
- CMake
- vcpkg
- Alpha Vantage API (data source)

---

## Features
- Stock price plotted over time
- Time window chosen by user (5 days, 1 month, 3 months, Full)
- Backend computes financial metrics:
  - Sharpe Ratio
  - Sortino Ratio
  - Maximum Drawdown
  - CAGR
- Stock data is cached in memory with TTL
- Demo data fallback when API fails
- Health endpoint to check server health

---

## API Endpoints
- `GET /api/market/history?symbol=AAPL&days=30`
- `GET /api/market/quote?symbol=AAPL`
- `GET /health`