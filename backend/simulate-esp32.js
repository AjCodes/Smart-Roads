// Simulates ESP32 sending sensor data continuously
// This script sends data every 3 seconds to test the live dashboard
// Run with: node simulate-esp32.js

const baseURL = 'http://localhost:5000';

// Generate random sensor data with realistic variations
function generateSensorData() {
  return {
    lane1: Math.floor(Math.random() * 150) + 20,  // 20-170 cm
    lane2: Math.floor(Math.random() * 200) + 50,  // 50-250 cm
    lane3: Math.floor(Math.random() * 100),       // 0-100 cm
    lane4: Math.floor(Math.random() * 180) + 30,  // 30-210 cm
    temperature: (Math.random() * 15 + 18).toFixed(1),  // 18-33°C
    humidity: (Math.random() * 40 + 40).toFixed(1),     // 40-80%
    pressure: (Math.random() * 30 + 1000).toFixed(1)    // 1000-1030 hPa
  };
}

// Send data to backend
async function sendData() {
  try {
    const data = generateSensorData();

    console.log(`\n📡 Sending data at ${new Date().toLocaleTimeString()}`);
    console.log('   Lanes:', `${data.lane1}cm, ${data.lane2}cm, ${data.lane3}cm, ${data.lane4}cm`);
    console.log('   Env:', `${data.temperature}°C, ${data.humidity}%, ${data.pressure}hPa`);

    const response = await fetch(`${baseURL}/api/sensor-data`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(data)
    });

    const result = await response.json();

    if (result.success) {
      console.log(`✅ Data saved successfully`);
      console.log(`🚦 Decision: ${result.decision.activeLane} for ${result.decision.duration}s`);
    } else {
      console.error('❌ Error:', result.error);
    }
  } catch (error) {
    console.error('❌ Connection error:', error.message);
  }
}

// Main loop
console.log('🚀 ESP32 Simulator Started');
console.log('📡 Sending data every 3 seconds...');
console.log('Press Ctrl+C to stop\n');

// Send initial data immediately
sendData();

// Then send every 3 seconds
setInterval(sendData, 3000);
