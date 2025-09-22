<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Live Sensor Data</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: sans-serif; padding: 20px; }
        .card { background: #f1f1f1; padding: 20px; border-radius: 10px; max-width: 300px; }
    </style>
</head>
<body>

    <h2>📡 Data Sensor (Realtime)</h2>
    <div class="card">
        <p><strong>Sensor A:</strong> <span id="sensor_a">--</span></p>
        <p><strong>Sensor B:</strong> <span id="sensor_b">--</span></p>
        <p><strong>Update Terakhir:</strong> <span id="updated_at">--</span></p>
    </div>

    <script>
        async function getData() {
            try {
                const response = await fetch("/api/sensor/latest");
                const data = await response.json();

                document.getElementById("sensor_a").textContent = data.sensor_a;
                document.getElementById("sensor_b").textContent = data.sensor_b;
                document.getElementById("updated_at").textContent = new Date(data.created_at).toLocaleString();
            } catch (error) {
                console.error("Gagal ambil data:", error);
            }
        }

        // Ambil data pertama kali
        getData();

        // Update tiap 3 detik
        setInterval(getData, 3000);
    </script>

</body>
</html>
