# 🚀 Backend API

Node.js backend for Smart Roads traffic system.

---

## ⚙️ Setup

**1. Install dependencies**
```bash
npm install
```

**2. Create `.env` file**
```env
PORT=5000
FIREBASE_DATABASE_URL=https://your-project.firebaseio.com
```

**3. Add Firebase key**
- Download `serviceAccountKey.json` from Firebase Console
- Place it in this folder

**4. Run server**
 ```bash
 npm run dev
 ```
 *Runs with nodemon for auto-restart on changes.*

✅ Server starts at `http://localhost:5000`

---

## 📡 API Endpoints

### 🤖 ESP32
```
POST /api/sensor-data          # Send sensor readings (lanes + environmental data)
GET /api/decision/latest       # Get AI decision
```

**Sensor Data Format:**
```json
{
  "lane1": 45,              // Distance in cm (required)
  "lane2": 120,             // Distance in cm (required)
  "lane3": 0,               // Distance in cm (required)
  "lane4": 0,               // Distance in cm (required)
  "temperature": 23.5,      // Temperature in °C (optional, BME280)
  "humidity": 65.2,         // Humidity in % (optional, BME280)
  "pressure": 1013.2        // Pressure in hPa (optional, BME280)
}
```

### 📊 Dashboard
```
GET /api/sensor-data           # Get sensor history
GET /api/decisions             # Get decision history
```

### 🔧 Other
```
GET /health                    # Check server status
POST /api/decision/generate    # Trigger AI analysis
```

---

## 🧪 Test

### Automated Testing

**Run all API tests:**
```bash
node test-api.js
```

**Simulate ESP32 continuous data (for dashboard testing):**
```bash
node simulate-esp32.js
```
This sends realistic sensor data every 3 seconds, perfect for testing the live dashboard.

### Manual Testing

**Send sensor data with environmental data:**
```powershell
Invoke-RestMethod -Uri "http://localhost:5000/api/sensor-data" -Method Post -ContentType "application/json" -Body '{"lane1":45.5,"lane2":120.3,"lane3":80.7,"lane4":200.0,"temperature":23.5,"humidity":65.2,"pressure":1013.2}'
```

**Get latest decision:**
```powershell
Invoke-RestMethod http://localhost:5000/api/decision/latest
```

---

## 🌐 Deploy

### Railway
```bash
railway login
railway init
railway up
```

**Add in Railway dashboard:**
- `NODE_ENV=production`
- `FIREBASE_DATABASE_URL=your-url`
- Upload `serviceAccountKey.json`

---

## 🐛 Troubleshooting

**Server won't start?**
- ✅ Check Node.js version (needs v18+)
- ✅ Make sure `.env` exists
- ✅ Make sure `serviceAccountKey.json` exists

**Can't connect to Firebase?**
- ✅ Check `FIREBASE_DATABASE_URL` in `.env`
- ✅ Check Firebase security rules allow read/write

---

## 👨‍💻 Developer

**AJ** - Backend & Database Developer

## 📊 Status

✅ **Complete and Production-Ready**

---

