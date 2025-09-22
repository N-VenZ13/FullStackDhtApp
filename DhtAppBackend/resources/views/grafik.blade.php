<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Speedometer Sensor</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/chartjs-plugin-doughnutlabel@1.0.3"></script>
    <script src="https://code.jquery.com/jquery-3.7.1.min.js"></script>
    <style>
        body {
            background: #f4f4f4;
            font-family: Arial, sans-serif;
            text-align: center;
            padding: 40px;
        }
        .chart-container {
            width: 300px;
            display: inline-block;
            margin: 20px;
        }
        h2 {
            margin-bottom: 30px;
        }
        .sensor-value {
            font-size: 20px;
            margin-top: 10px;
            color: #333;
        }
    </style>
</head>
<body>
    <h2>Speedometer Realtime Sensor A dan B</h2>

    <div class="chart-container">
        <canvas id="gaugeA"></canvas>
        <p><strong>Sensor A</strong></p>
        <p class="sensor-value" id="valueA">Nilai: 0</p>
    </div>
    <div class="chart-container">
        <canvas id="gaugeB"></canvas>
        <p><strong>Sensor B</strong></p>
        <p class="sensor-value" id="valueB">Nilai: 0</p>
    </div>

    <script>
        const maxVal = 100;

        const configGauge = (label, color) => ({
            type: 'doughnut',
            data: {
                labels: [label],
                datasets: [{
                    data: [0, maxVal],
                    backgroundColor: [color, "#e0e0e0"],
                    borderWidth: 0
                }]
            },
            options: {
                rotation: -90,
                circumference: 180,
                cutout: '80%',
                plugins: {
                    doughnutlabel: {
                        labels: [
                            {
                                text: '0',
                                font: {
                                    size: '28'
                                }
                            },
                            {
                                text: label
                            }
                        ]
                    },
                    legend: { display: false }
                }
            }
        });

        const gaugeA = new Chart(document.getElementById('gaugeA'), configGauge("Sensor A", "#FF6384"));
        const gaugeB = new Chart(document.getElementById('gaugeB'), configGauge("Sensor B", "#36A2EB"));

        function updateGauge(chart, value, labelId) {
            chart.data.datasets[0].data[0] = value;
            chart.data.datasets[0].data[1] = maxVal - value;
            chart.options.plugins.doughnutlabel.labels[0].text = value.toString();
            chart.update();
            document.getElementById(labelId).innerText = `Nilai: ${value}`;
        }

        function fetchData() {
            $.get('/api/sensor/latest', function(data) {
                updateGauge(gaugeA, data.sensor_a, 'valueA');
                updateGauge(gaugeB, data.sensor_b, 'valueB');
            });
        }

        setInterval(fetchData, 2000);
    </script>
</body>
</html>
