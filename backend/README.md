# Backend API

Node.js backend for Smart Roads traffic system.

## Setup

**Install dependencies:**
```bash
npm install
```

**Create `.env` file:**
```env
PORT=5000
FIREBASE_DATABASE_URL=https://your-project.firebaseio.com
```

**Add Firebase credentials:**
- Download `serviceAccountKey.json` from Firebase Console
- Place it in the backend folder

**Run server:**
```bash
npm run dev
```

Server runs at `http://localhost:5000`

---

## API Endpoints

**ESP32:**
- `POST /api/sensor-data` - Send sensor readings
- `GET /api/decision/latest` - Get latest traffic decision

**Dashboard:**
- `GET /api/sensor-data` - Get sensor history
- `GET /api/sensor-data/latest` - Get latest sensor data
- `GET /api/decisions` - Get decision history

**Other:**
- `GET /health` - Server health check
- `POST /api/decision/generate` - Manually trigger decision

**Sensor Data Format:**
```json
{
  "lane1": {"carCount": 3, "firstTriggered": 1234567890},
  "lane2": {"carCount": 0, "firstTriggered": 0},
  "lane3": {"carCount": 1, "firstTriggered": 1234567900},
  "lane4": {"carCount": 5, "firstTriggered": 1234567850},
  "temperature": 23.5,
  "humidity": 65.2,
  "pressure": 1013.2
}
```

## Testing

**Run API tests:**
```bash
node test-api.js
```

**Simulate ESP32:**
```bash
node simulate-esp32.js
```

**Clear Firebase data:**
```bash
node clear-firebase.js
```

## Decision Algorithm

The system analyzes sensor data and makes decisions:
1. Prioritizes lane with most cars
2. If tied, selects lane waiting longest
3. Assigns green light duration (10 seconds)
4. Returns decision to ESP32

## Troubleshooting

- Check Node.js version (requires v18+)
- Verify `.env` file exists
- Ensure `serviceAccountKey.json` is in place
- Check Firebase database URL is correct

---

