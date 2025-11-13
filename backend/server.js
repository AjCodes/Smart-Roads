require('dotenv').config();
const express = require('express');
const cors = require('cors');

const app = express();
const PORT = process.env.PORT || 5000;

app.use(cors());
app.use(express.json());

app.get('/health', (req, res) => {
  res.json({ status: 'OK', message: 'Backend running' });
});

// TODO: Add routes here

app.listen(PORT, () => {
  console.log(`🚀 Server running on port ${PORT}`);
});
