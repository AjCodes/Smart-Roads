import 'dotenv/config';
import express from 'express';
import cors from 'cors';

import './config/firebase.js';  // This line executes firebase.js

// Import routes
import sensorRoutes from './routes/sensorRoutes.js';
import decisionRoutes from './routes/decisionRoutes.js';

const app = express();
const PORT = process.env.PORT || 5000;

// Middleware
app.use(cors());
app.use(express.json());

// Health check endpoint
app.get('/health', (req, res) => {
  res.json({ 
    status: 'OK', 
    message: 'Smart Roads Backend is running',
    timestamp: new Date().toISOString()
  });
});

// API Routes
app.use('/api/sensor-data', sensorRoutes);
app.use('/api/decision', decisionRoutes);
app.use('/api/decisions', decisionRoutes); // Alternative endpoint for fetching decisions

// 404 handler
app.use((req, res) => {
  res.status(404).json({
    success: false,
    error: 'Endpoint not found'
  });
});

// Error handler
app.use((err, req, res, next) => {
  console.error('❌ Unhandled error:', err);
  res.status(500).json({
    success: false,
    error: 'Internal server error',
    details: err.message
  });
});

app.listen(PORT, () => {
  console.log(`🚀 Server running on port ${PORT}`);
  console.log(`📍 Health check: http://localhost:${PORT}/health`);
  console.log(`📊 Sensor data API: http://localhost:${PORT}/api/sensor-data`);
  console.log(`🧠 Decision API: http://localhost:${PORT}/api/decision`);
});