import express from 'express';
import {
  generateDecision,
  getLatestDecision,
  getDecisions
} from '../controllers/decisionController.js';

const router = express.Router();

// POST /api/decision/generate - Trigger system to make a decision
router.post('/generate', generateDecision);

// GET /api/decision/latest - Arduino fetches the latest decision
router.get('/latest', getLatestDecision);

// GET /api/decisions - Dashboard fetches decision history
router.get('/', getDecisions);

export default router;