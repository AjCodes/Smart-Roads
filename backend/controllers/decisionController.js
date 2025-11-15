import { db } from '../config/firebase.js';

// Simple AI Logic: Give green light to the lane with most traffic (shortest distance)
const analyzeTrafficAndDecide = (sensorData) => {
  const { lane1, lane2, lane3, lane4 } = sensorData;

  const lanes = [
    { name: 'lane1', distance: lane1 },
    { name: 'lane2', distance: lane2 },
    { name: 'lane3', distance: lane3 },
    { name: 'lane4', distance: lane4 }
  ];

  // Sort by distance (ascending) - shortest distance = most traffic
  lanes.sort((a, b) => a.distance - b.distance);

  const priorityLane = lanes[0].name;

  // Calculate duration based on traffic density
  let duration = 30; // Default 30 seconds

  if (lanes[0].distance < 50) {
    duration = 45; // Heavy traffic: 45 seconds
  } else if (lanes[0].distance < 100) {
    duration = 35; // Moderate traffic: 35 seconds
  }

  return {
    activeLane: priorityLane,
    duration: duration,
    reason: `Lane with shortest distance (${lanes[0].distance}cm) indicating most traffic`,
    trafficDensity: lanes[0].distance < 50 ? 'heavy' : lanes[0].distance < 100 ? 'moderate' : 'light'
  };
};

// POST /api/decision/generate - Generate new decision based on latest sensor data
export const generateDecision = async (req, res) => {
  try {
    // Get the latest sensor data
    const snapshot = await db.ref('sensor_data')
      .orderByChild('timestamp')
      .limitToLast(1)
      .once('value');

    if (!snapshot.exists()) {
      return res.status(404).json({
        success: false,
        error: 'No sensor data available to make decision'
      });
    }

    let latestSensorData = null;
    snapshot.forEach((child) => {
      latestSensorData = child.val();
    });

    // Run AI analysis
    const decision = analyzeTrafficAndDecide(latestSensorData);
    
    decision.timestamp = Date.now();
    decision.createdAt = new Date().toISOString();

    // Save decision to database
    await db.ref('decisions').push(decision);

    console.log('🧠 AI Decision made:', decision);

    res.status(201).json({
      success: true,
      message: 'Decision generated successfully',
      decision
    });

  } catch (error) {
    console.error('❌ Error generating decision:', error);
    res.status(500).json({
      success: false,
      error: 'Failed to generate decision',
      details: error.message
    });
  }
};

// GET /api/decision/latest - Get the latest decision for Arduino
export const getLatestDecision = async (req, res) => {
  try {
    const snapshot = await db.ref('decisions')
      .orderByChild('timestamp')
      .limitToLast(1)
      .once('value');

    if (!snapshot.exists()) {
      return res.status(404).json({
        success: false,
        message: 'No decisions found'
      });
    }

    let latestDecision = null;
    snapshot.forEach((child) => {
      latestDecision = {
        id: child.key,
        ...child.val()
      };
    });

    res.json({
      success: true,
      decision: latestDecision
    });

  } catch (error) {
    console.error('❌ Error fetching latest decision:', error);
    res.status(500).json({
      success: false,
      error: 'Failed to fetch latest decision',
      details: error.message
    });
  }
};

// GET /api/decisions - Get decision history (for dashboard)
export const getDecisions = async (req, res) => {
  try {
    const limit = parseInt(req.query.limit) || 50;

    const snapshot = await db.ref('decisions')
      .orderByChild('timestamp')
      .limitToLast(limit)
      .once('value');

    if (!snapshot.exists()) {
      return res.json({
        success: true,
        data: [],
        message: 'No decisions found'
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
    console.error('❌ Error fetching decisions:', error);
    res.status(500).json({
      success: false,
      error: 'Failed to fetch decisions',
      details: error.message
    });
  }
};