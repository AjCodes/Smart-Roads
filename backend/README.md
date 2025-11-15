\# Backend API



Node.js backend for Smart Roads traffic system.



---



\## Setup



1\. \*\*Install dependencies\*\*

```bash

npm install

```



2\. \*\*Create `.env` file\*\*

```env

PORT=5000

FIREBASE\_DATABASE\_URL=https://your-project.firebaseio.com

```



3\. \*\*Add Firebase key\*\*

&nbsp;  - Download `serviceAccountKey.json` from Firebase Console

&nbsp;  - Place it in this folder



4\. \*\*Run server\*\*

```bash

npm run dev

```



Server starts at `http://localhost:5000`



---



\## API Endpoints



\### Arduino

```

POST /api/sensor-data          # Send sensor readings

GET /api/decision/latest       # Get AI decision

```



\### Dashboard

```

GET /api/sensor-data           # Get sensor history

GET /api/decisions             # Get decision history

```



\### Other

```

GET /health                    # Check server status

POST /api/decision/generate    # Trigger AI analysis

```



---



\## Test



\*\*Send sensor data:\*\*

```powershell

Invoke-RestMethod -Uri "http://localhost:5000/api/sensor-data" -Method Post -ContentType "application/json" -Body '{"lane1":45.5,"lane2":120.3,"lane3":80.7,"lane4":200.0}'

```



\*\*Get latest decision:\*\*

```powershell

Invoke-RestMethod http://localhost:5000/api/decision/latest

```



---



\## Deploy



\### Railway

```bash

railway login

railway init

railway up

```



Add these in Railway dashboard:

\- `NODE\_ENV=production`

\- `FIREBASE\_DATABASE\_URL=your-url`

\- Upload `serviceAccountKey.json`



---



\## Troubleshooting



\*\*Server won't start?\*\*

\- Check Node.js version (needs v18+)

\- Make sure `.env` exists

\- Make sure `serviceAccountKey.json` exists



\*\*Can't connect to Firebase?\*\*

\- Check `FIREBASE\_DATABASE\_URL` in `.env`

\- Check Firebase security rules allow read/write



---



\*\*Developer:\*\* AJ  

\*\*Status:\*\* ✅ Complete



