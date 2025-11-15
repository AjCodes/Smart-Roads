// Middleware to validate sensor data from Arduino
export const validateSensorData = (req, res, next) => {
  const { lane1, lane2, lane3, lane4 } = req.body;

  // Check if all required fields are present
  if (lane1 === undefined || lane2 === undefined || lane3 === undefined || lane4 === undefined) {
    return res.status(400).json({
      success: false,
      error: 'Missing required fields. Expected: lane1, lane2, lane3, lane4'
    });
  }

  // Validate that all values are numbers
  const lanes = [lane1, lane2, lane3, lane4];
  for (let i = 0; i < lanes.length; i++) {
    if (typeof lanes[i] !== 'number' || isNaN(lanes[i])) {
      return res.status(400).json({
        success: false,
        error: `Invalid value for lane${i + 1}. Must be a number.`
      });
    }
  }

  // Validate that all values are within acceptable range (0-400cm for ultrasonic sensors)
  for (let i = 0; i < lanes.length; i++) {
    if (lanes[i] < 0 || lanes[i] > 400) {
      return res.status(400).json({
        success: false,
        error: `Invalid value for lane${i + 1}. Must be between 0 and 400cm.`
      });
    }
  }

  // Data is valid, proceed to next middleware
  next();
};