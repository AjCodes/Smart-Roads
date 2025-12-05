import { db } from '../config/firebase.js';

// AI Logic: Give green light to the lane with most cars (count-based priority)
export const analyzeTrafficAndDecide = (sensorData) => {
  const lanes = [
    { name: 'lane1', count: sensorData.lane1?.carCount || 0, first: sensorData.lane1?.firstTriggered || 0 },
    { name: 'lane2', count: sensorData.lane2?.carCount || 0, first: sensorData.lane2?.firstTriggered || 0 },
    { name: 'lane3', count: sensorData.lane3?.carCount || 0, first: sensorData.lane3?.firstTriggered || 0 },
    { name: 'lane4', count: sensorData.lane4?.carCount || 0, first: sensorData.lane4?.firstTriggered || 0 }
  ];

  // Filter lanes with at least 1 car waiting
  const activeLanes = lanes.filter(l => l.count > 0);

  if (activeLanes.length === 0) {
    return {
      activeLane: 'none',
      resetLane: 0,
      duration: 0,
      reason: 'No cars detected on any lane',
      trafficDensity: 'none'
    };
  }

  // Sort by:
  // 1. Car count (descending) - more cars = higher priority
  // 2. First triggered time (ascending) - earlier = wins tie
  activeLanes.sort((a, b) => {
    if (b.count !== a.count) return b.count - a.count;
    return a.first - b.first;  // Earlier timestamp wins
  });

  const winner = activeLanes[0];
  const resetLane = parseInt(winner.name.replace('lane', '')); // "lane1" -> 1

  // Calculate traffic density
  let trafficDensity;
  if (winner.count >= 9) {
    trafficDensity = 'heavy';
  } else if (winner.count >= 5) {
    trafficDensity = 'moderate';
  } else {
    trafficDensity = 'light';
  }

  // Dynamic duration based on car count
  let duration = 10; // Fixed 10 seconds

  return {
    activeLane: winner.name,
    resetLane: resetLane,
    duration: duration,
    reason: `${winner.name} has ${winner.count} car(s) waiting`,
    trafficDensity: trafficDensity,
    allLaneCounts: {
      lane1: lanes[0].count,
      lane2: lanes[1].count,
      lane3: lanes[2].count,
      lane4: lanes[3].count
    }
  };
};

// POST /api/decision/generate - Generate new decision based on latest sensor data
export const generateDecision = async (req, res) => {
  try {
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

    const decision = analyzeTrafficAndDecide(latestSensorData);
    decision.timestamp = Date.now();
    decision.createdAt = new Date().toISOString();

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