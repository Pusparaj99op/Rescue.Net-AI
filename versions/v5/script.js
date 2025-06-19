
document.addEventListener('DOMContentLoaded', () => {
    // Replace with your WebSocket server address
    const WEBSOCKET_URL = 'ws://' + window.location.host + '/ws';
    let socket;

    const heartRateEl = document.getElementById('heart-rate');
    const temperatureEl = document.getElementById('temperature');
    const fallDetectionEl = document.getElementById('fall-detection');
    const batteryLevelEl = document.getElementById('battery-level');
    const emergencyBtn = document.getElementById('emergency-btn');

    let map;
    let marker;

    function initMap() {
        map = new google.maps.Map(document.getElementById('map'), {
            center: { lat: -34.397, lng: 150.644 },
            zoom: 8
        });
        marker = new google.maps.Marker({
            map: map,
            position: { lat: -34.397, lng: 150.644 },
            title: 'User Location'
        });
    }

    function connectWebSocket() {
        socket = new WebSocket(WEBSOCKET_URL);

        socket.onopen = () => {
            console.log('WebSocket connection established');
        };

        socket.onmessage = (event) => {
            const data = JSON.parse(event.data);
            updateDashboard(data);
        };

        socket.onclose = () => {
            console.log('WebSocket connection closed. Reconnecting...');
            setTimeout(connectWebSocket, 3000);
        };

        socket.onerror = (error) => {
            console.error('WebSocket error:', error);
        };
    }

    function updateDashboard(data) {
        if (data.heart_rate) heartRateEl.textContent = data.heart_rate.toFixed(1);
        if (data.temperature) temperatureEl.textContent = data.temperature.toFixed(1);
        if (data.fall_detected) fallDetectionEl.textContent = data.fall_detected ? 'Yes' : 'No';
        if (data.battery) batteryLevelEl.textContent = data.battery;

        if (data.lat && data.lng) {
            const newPos = { lat: data.lat, lng: data.lng };
            map.setCenter(newPos);
            marker.setPosition(newPos);
        }

        // Update chart
        updateVitalsChart(data);
    }

    emergencyBtn.addEventListener('click', () => {
        if (confirm('Are you sure you want to trigger a manual emergency?')) {
            // Send emergency signal via WebSocket or HTTP POST
            console.log('Emergency triggered');
            // Example: socket.send(JSON.stringify({ emergency: true }));
            // Or use fetch to call the /api/emergency endpoint
            fetch('/api/emergency', { method: 'POST' });
        }
    });

    // Chart.js setup
    const ctx = document.getElementById('vitalsChart').getContext('2d');
    const vitalsChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [], // Timestamps
            datasets: [{
                label: 'Heart Rate (BPM)',
                data: [],
                borderColor: 'rgb(255, 99, 132)',
                tension: 0.1
            }, {
                label: 'Temperature (°C)',
                data: [],
                borderColor: 'rgb(54, 162, 235)',
                tension: 0.1
            }]
        },
        options: {
            scales: {
                x: {
                    type: 'time',
                    time: {
                        unit: 'minute'
                    }
                }
            }
        }
    });

    function updateVitalsChart(data) {
        const now = new Date();
        vitalsChart.data.labels.push(now);
        vitalsChart.data.datasets[0].data.push(data.heart_rate);
        vitalsChart.data.datasets[1].data.push(data.temperature);

        // Limit data points
        if (vitalsChart.data.labels.length > 50) {
            vitalsChart.data.labels.shift();
            vitalsChart.data.datasets.forEach(dataset => {
                dataset.data.shift();
            });
        }
        vitalsChart.update();
    }

    initMap();
    // connectWebSocket(); // Uncomment when WebSocket server is ready
});
