import { db } from '../config/firebase.js';
import { analyzeTrafficAndDecide } from './decisionController.js';

// POST /api/sensor-data - Receive sensor data from Arduino
export const saveSensorData = async (req, res) => {
  try {
    const { lane1, lane2, lane3, lane4 } = req.body;
    const timestamp = Date.now();

    // Prepare data object
    const sensorData = {
      lane1,
      lane2,
      lane3,
      lane4,
      timestamp,
      createdAt: new Date().toISOString()
    };

    // Save to Firebase Realtime Database
    await db.ref('sensor_data').push(sensorData);

    console.log('📊 Sensor data saved:', sensorData);

    // --- AI DECISION LOGIC ---
    // Run AI analysis immediately
    const decision = analyzeTrafficAndDecide(sensorData);

    decision.timestamp = Date.now();
    decision.createdAt = new Date().toISOString();

    // Save decision to Firebase
    await db.ref('decisions').push(decision);

    console.log('🧠 AI Decision made immediately:', decision);

    res.status(201).json({
      success: true,
      message: 'Sensor data saved and decision generated',
      data: sensorData,
      decision: decision // Return decision immediately
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

// GET /api/sensor-data - Get recent sensor data (for dashboard)
export const getSensorData = async (req, res) => {
  try {
    const limit = parseInt(req.query.limit) || 50;

    const snapshot = await db.ref('sensor_data')
      .orderByChild('timestamp')
      .limitToLast(limit)
      .once('value');

    if (!snapshot.exists()) {
      return res.json({
        success: true,
        data: [],
        message: 'No sensor data found'
      });
    }

    const data = [];
    snapshot.forEach((child) => {
      data.push({
        id: child.key,
        ...child.val()
      });
    });

    res.json({
      success: true,
      count: data.length,
      data: data.reverse()
    });

  } catch (error) {
    console.error('❌ Error fetching sensor data:', error);
    res.status(500).json({
      success: false,
      error: 'Failed to fetch sensor data',
      details: error.message
    });
  }
};

// GET /api/sensor-data/latest - Get the most recent sensor reading
export const getLatestSensorData = async (req, res) => {
  try {
    const snapshot = await db.ref('sensor_data')
      .orderByChild('timestamp')
      .limitToLast(1)
      .once('value');

    if (!snapshot.exists()) {
      return res.status(404).json({
        success: false,
        message: 'No sensor data found'
      });
    }

    let latestData = null;
    snapshot.forEach((child) => {
      latestData = {
        id: child.key,
        ...child.val()
      };
    });

    res.json({
      success: true,
      data: latestData
    });

  } catch (error) {
    console.error('❌ Error fetching latest sensor data:', error);
    res.status(500).json({
      success: false,
      error: 'Failed to fetch latest sensor data',
      details: error.message
    });
  }
};