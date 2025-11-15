import express from 'express';
import { validateSensorData } from '../middleware/validation.js';
import {
  saveSensorData,
  getSensorData,
  getLatestSensorData
} from '../controllers/sensorController.js';

const router = express.Router();

// POST /api/sensor-data - Arduino sends sensor readings
router.post('/', validateSensorData, saveSensorData);

// GET /api/sensor-data - Dashboard fetches sensor history
router.get('/', getSensorData);

// GET /api/sensor-data/latest - Get most recent reading
router.get('/latest', getLatestSensorData);

export default router;