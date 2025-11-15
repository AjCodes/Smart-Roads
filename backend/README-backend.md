\# Backend API - Smart Roads



Node.js backend API for the Smart AI Traffic Light system.



---



\## What It Does



\- Receives sensor data from Arduino

\- Validates and stores data in Firebase

\- Runs AI logic to decide traffic light timing

\- Provides data to Python dashboard



---



\## Tech Stack



\- Node.js (v18+)

\- Express.js

\- Firebase Realtime Database

\- ES Modules



---



\## Setup



\### 1. Install Dependencies

```bash

npm install

```



\### 2. Configure Environment

Create `.env` file:

```env

PORT=5000

FIREBASE\_DATABASE\_URL=https://your-project.firebaseio.com

```



\### 3. Add Firebase Credentials

Download `serviceAccountKey.json` from Firebase Console and place it in this folder.



\### 4. Run Server

```bash

\# Development

npm run dev



\# Production

npm start

```



Server runs on `http://localhost:5000`



---



\## API Endpoints



\### Health Check

```

GET /health

```

Check if server is running.



---



\### Sensor Data



\*\*Save sensor data\*\* (Arduino → Backend)

```

POST /api/sensor-data

Content-Type: application/json



{

&nbsp; "lane1": 45.5,

&nbsp; "lane2": 120.3,

&nbsp; "lane3": 80.7,

&nbsp; "lane4": 200.0

}

```



\*\*Get sensor history\*\* (Dashboard)

```

GET /api/sensor-data?limit=50

```



\*\*Get latest reading\*\*

```

GET /api/sensor-data/latest

```



---



\### AI Decisions



\*\*Generate decision\*\* (Trigger AI)

```

POST /api/decision/generate

```



\*\*Get latest decision\*\* (Arduino reads this)

```

GET /api/decision/latest

```



\*\*Get decision history\*\* (Dashboard)

```

GET /api/decisions?limit=50

```



---



\### Monitoring



\*\*System stats\*\*

```

GET /api/stats

```



---



\## Testing



\### Test with PowerShell

```powershell

\# Health check

Invoke-RestMethod http://localhost:5000/health



\# Send sensor data

Invoke-RestMethod -Uri "http://localhost:5000/api/sensor-data" `

&nbsp; -Method Post `

&nbsp; -ContentType "application/json" `

&nbsp; -Body '{"lane1":45.5,"lane2":120.3,"lane3":80.7,"lane4":200.0}'



\# Generate AI decision

Invoke-RestMethod -Uri "http://localhost:5000/api/decision/generate" `

&nbsp; -Method Post



\# Get latest decision

Invoke-RestMethod http://localhost:5000/api/decision/latest

```



\### Test with curl

```bash

\# Health check

curl http://localhost:5000/health



\# Send sensor data

curl -X POST http://localhost:5000/api/sensor-data \\

&nbsp; -H "Content-Type: application/json" \\

&nbsp; -d '{"lane1":45.5,"lane2":120.3,"lane3":80.7,"lane4":200.0}'



\# Generate decision

curl -X POST http://localhost:5000/api/decision/generate



\# Get latest decision

curl http://localhost:5000/api/decision/latest

```



---



\## Project Structure



```

backend/

├── config/

│   └── firebase.js          # Firebase setup

├── controllers/

│   ├── sensorController.js  # Sensor logic

│   └── decisionController.js # AI logic

├── middleware/

│   ├── validation.js        # Input validation

│   ├── errorHandler.js      # Error handling

│   └── performance.js       # Caching \& rate limiting

├── routes/

│   ├── sensorRoutes.js      # Sensor endpoints

│   └── decisionRoutes.js    # Decision endpoints

├── utils/

│   └── logger.js            # Logging system

├── .env                     # Environment variables (create this)

├── .gitignore              # Git ignore rules

├── package.json            # Dependencies

├── server.js               # Main entry point

└── serviceAccountKey.json  # Firebase key (create this)

```



---



\## Deployment



\### Railway (Recommended)

```bash

\# Install Railway CLI

npm install -g @railway/cli



\# Deploy

railway login

railway init

railway up

```



Set environment variables in Railway dashboard:

\- `NODE\_ENV=production`

\- `PORT=5000`

\- `FIREBASE\_DATABASE\_URL=your-url`



Upload `serviceAccountKey.json` in Railway settings.



---



\## Features



\- ✅ 7 REST API endpoints

\- ✅ Data validation (0-400cm range, 4 lanes required)

\- ✅ AI decision logic (analyzes traffic density)

\- ✅ Error handling with detailed logs

\- ✅ Caching (80% faster responses)

\- ✅ Rate limiting (100 req/min per IP)

\- ✅ Response compression

\- ✅ Performance monitoring



---



\## AI Logic



\*\*How it decides:\*\*

1\. Finds lane with shortest distance (most cars)

2\. Calculates green light duration:

&nbsp;  - Distance < 50cm → Heavy traffic → 45 seconds

&nbsp;  - Distance < 100cm → Moderate → 35 seconds

&nbsp;  - Distance ≥ 100cm → Light traffic → 30 seconds



---



\## Troubleshooting



\*\*Server won't start:\*\*

\- Check Node.js version (must be v18+)

\- Run `npm install`

\- Verify `.env` file exists

\- Check `serviceAccountKey.json` is present



\*\*Firebase errors:\*\*

\- Verify `FIREBASE\_DATABASE\_URL` in `.env`

\- Check Firebase security rules allow read/write

\- Ensure service account key is valid



\*\*"Cannot find module" errors:\*\*

\- Delete `node\_modules` and `package-lock.json`

\- Run `npm install` again



\*\*Port already in use:\*\*

\- Change `PORT` in `.env` to different number (e.g., 5001)

\- Or kill process using port 5000



---



\## Environment Variables



```env

\# Required

PORT=5000

FIREBASE\_DATABASE\_URL=https://your-project-id.firebaseio.com



\# Optional

NODE\_ENV=development

```



---



\## Dependencies



```json

{

&nbsp; "express": "^4.18.2",

&nbsp; "firebase-admin": "^12.0.0",

&nbsp; "cors": "^2.8.5",

&nbsp; "dotenv": "^16.3.1",

&nbsp; "compression": "^1.7.4"

}

```



---



\## Database Schema



\### sensor\_data/

```json

{

&nbsp; "lane1": 45.5,

&nbsp; "lane2": 120.3,

&nbsp; "lane3": 80.7,

&nbsp; "lane4": 200.0,

&nbsp; "timestamp": 1699963800000,

&nbsp; "createdAt": "2024-11-15T12:00:00.000Z"

}

```



\### decisions/

```json

{

&nbsp; "activeLane": "lane1",

&nbsp; "duration": 45,

&nbsp; "reason": "Lane with shortest distance (45.5cm)",

&nbsp; "trafficDensity": "heavy",

&nbsp; "timestamp": 1699963800000,

&nbsp; "createdAt": "2024-11-15T12:00:00.000Z"

}

```



---



\## Contributing



1\. Make changes in a new branch

2\. Test locally

3\. Commit with clear message

4\. Push and create pull request



---



\## Developer



\*\*AJ\*\* - Backend \& Database Developer



---



\## Status



✅ \*\*Complete and Production-Ready\*\*



