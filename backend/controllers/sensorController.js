import { db } from '../config/firebase.js';
import { analyzeTrafficAndDecide } from './decisionController.js';

// Track active traffic cycle to prevent repeated decisions
let currentCycleActive = false;
let currentCycleLane = null;
let cycleStartTime = 0;
const CYCLE_DURATION = 13000; // 10s green + 3s yellow

// POST /api/sensor-data - Receive sensor data from ESP32
export const saveSensorData = async (req, res) => {
  try {
    const { lane1, lane2, lane3, lane4, temperature, humidity, pressure } = req.body;
    const timestamp = Date.now();

    // Prepare data object with nested lane structure
    const sensorData = {
      lane1: { carCount: lane1.carCount, firstTriggered: lane1.firstTriggered },
      lane2: { carCount: lane2.carCount, firstTriggered: lane2.firstTriggered },
      lane3: { carCount: lane3.carCount, firstTriggered: lane3.firstTriggered },
      lane4: { carCount: lane4.carCount, firstTriggered: lane4.firstTriggered },
      timestamp,
      createdAt: new Date().toISOString()
    };

    // Add environmental data if provided
    if (temperature !== undefined) sensorData.temperature = temperature;
    if (humidity !== undefined) sensorData.humidity = humidity;
    if (pressure !== undefined) sensorData.pressure = pressure;

    // Save to Firebase
    await db.ref('sensor_data').push(sensorData);

    // Check if current cycle has expired
    if (currentCycleActive && (Date.now() - cycleStartTime > CYCLE_DURATION)) {
      console.log('⏱️ Previous cycle expired, ready for new decision');
      currentCycleActive = false;
      currentCycleLane = null;
    }

    // --- AI DECISION LOGIC ---
    let decision;

    if (currentCycleActive) {
      // Cycle in progress - don't send new decision
      console.log(`🚦 Cycle active for ${currentCycleLane}, waiting...`);
      decision = {
        activeLane: 'none',
        resetLane: 0,
        duration: 0,
        reason: `Waiting for ${currentCycleLane} cycle to complete`,
        trafficDensity: 'none',
        cycleInProgress: true
      };
    } else {
      // No active cycle - make a decision
      decision = analyzeTrafficAndDecide(sensorData);

      // If we decided to give green to a lane, start the cycle
      if (decision.activeLane !== 'none') {
        currentCycleActive = true;
        currentCycleLane = decision.activeLane;
        cycleStartTime = Date.now();
        console.log(`🚦 Starting new cycle for ${decision.activeLane}`);
      }
    }

    decision.timestamp = Date.now();
    decision.createdAt = new Date().toISOString();

    // Save decision to Firebase
    await db.ref('decisions').push(decision);

    console.log('🧠 AI Decision:', decision.activeLane, '- Reason:', decision.reason);

    res.status(201).json({
      success: true,
      message: 'Sensor data saved',
      data: sensorData,
      decision: decision,
      environmental: { temperature, humidity, pressure }
    });

  } catch (error) {
    console.error('❌ Error processing sensor data:', error);
    res.status(500).json({
      success: false,
      error: 'Failed to process sensor data',
      details: error.message
    });
  }
};

// GET /api/sensor-data - Get recent sensor data
export const getSensorData = async (req, res) => {
  try {
    const limit = parseInt(req.query.limit) || 50;

    const snapshot = await db.ref('sensor_data')
      .orderByChild('timestamp')
      .limitToLast(limit)
      .once('value');

    if (!snapshot.exists()) {
      return res.json({ success: true, data: [], message: 'No sensor data found' });
    }

    const data = [];
    snapshot.forEach((child) => {
      data.push({ id: child.key, ...child.val() });
    });

    res.json({ success: true, count: data.length, data: data.reverse() });

  } catch (error) {
    console.error('❌ Error fetching sensor data:', error);
    res.status(500).json({ success: false, error: 'Failed to fetch sensor data' });
  }
};

// GET /api/sensor-data/latest
export const getLatestSensorData = async (req, res) => {
  try {
    const snapshot = await db.ref('sensor_data')
      .orderByChild('timestamp')
      .limitToLast(1)
      .once('value');

    if (!snapshot.exists()) {
      return res.status(404).json({ success: false, message: 'No sensor data found' });
    }

    let latestData = null;
    snapshot.forEach((child) => {
      latestData = { id: child.key, ...child.val() };
    });

    res.json({ success: true, data: latestData });

  } catch (error) {
    console.error('❌ Error fetching latest sensor data:', error);
    res.status(500).json({ success: false, error: 'Failed to fetch latest sensor data' });
  }
};