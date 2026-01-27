const API_KEY = process.env.ALPHA_VANTAGE_API_KEY;

export async function getDailyHistory(symbol) {
    const url = `https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=${symbol}&apikey=${API_KEY}`;

    const response = await fetch(url);
    const data = await response.json();

    return data;
}