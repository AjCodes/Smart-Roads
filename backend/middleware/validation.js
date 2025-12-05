// Middleware to validate sensor data from ESP32

export const validateSensorData = (req, res, next) => {
  const { lane1, lane2, lane3, lane4, temperature, humidity, pressure } = req.body;

  // Validate lane data structure (nested objects with carCount and firstTriggered)
  const lanes = [
    { data: lane1, name: 'lane1' },
    { data: lane2, name: 'lane2' },
    { data: lane3, name: 'lane3' },
    { data: lane4, name: 'lane4' }
  ];

  for (const lane of lanes) {
    if (!lane.data || typeof lane.data !== 'object') {
      return res.status(400).json({
        success: false,
        error: `Missing or invalid ${lane.name}. Expected: { carCount: number, firstTriggered: number }`
      });
    }

    if (typeof lane.data.carCount !== 'number' || lane.data.carCount < 0) {
      return res.status(400).json({
        success: false,
        error: `Invalid carCount for ${lane.name}. Must be a non-negative number.`
      });
    }

    if (typeof lane.data.firstTriggered !== 'number') {
      return res.status(400).json({
        success: false,
        error: `Invalid firstTriggered for ${lane.name}. Must be a number (timestamp).`
      });
    }
  }

  // Validate optional BME280 environmental sensor data
  if (temperature !== undefined) {
    if (typeof temperature !== 'number' || isNaN(temperature)) {
      return res.status(400).json({
        success: false,
        error: 'Invalid temperature value. Must be a number.'
      });
    }
  }

  if (humidity !== undefined) {
    if (typeof humidity !== 'number' || isNaN(humidity)) {
      return res.status(400).json({
        success: false,
        error: 'Invalid humidity value. Must be a number.'
      });
    }
  }

  if (pressure !== undefined) {
    if (typeof pressure !== 'number' || isNaN(pressure)) {
      return res.status(400).json({
        success: false,
        error: 'Invalid pressure value. Must be a number.'
      });
    }
  }

  // Data is valid, proceed to next middleware
  next();
};