import {
    Chart as ChartJS,
    CategoryScale,
    LinearScale,
    PointElement,
    LineElement,
    Tooltip,
    Legend,
} from "chart.js";
import { Line } from "react-chartjs-2";

// Register required Chart.js components
ChartJS.register(
    CategoryScale,
    LinearScale,
    PointElement,
    LineElement,
    Tooltip,
    Legend
);

export default function StockChart({ series }) {
    // Expecting series like:
    // [{ date: "2026-01-27", price: 252.10 }, ...]

    const labels = series.map((point) => point.date);
    const prices = series.map((point) => point.price);

    const data = {
        labels,
        datasets: [
            {
            label: "Price",
            data: prices,
            borderColor: "rgb(75, 192, 192)",
            backgroundColor: "rgba(75, 192, 192, 0.2)",
            tension: 0.2,
            },
        ],
    };

    const options = {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
            legend: {
            display: true,
            },
        },
        scales: {
            y: {
            beginAtZero: false,
            },
        },
    };

    return (
        <div style={{ height: "400px" }}>
            <Line data={data} options={options} />
        </div>
    );
}
