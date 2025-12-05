# Dashboard

Python Flask dashboard for Smart Roads traffic visualization.

## Setup

**Install dependencies:**
```bash
pip install -r requirements.txt
```

**Run dashboard:**
```bash
python app.py
```

Dashboard starts at `http://localhost:5001`

## Features

- Real-time car count visualization for all 4 lanes
- Live traffic light status display
- Environmental data (temperature, humidity, pressure)
- Auto-refresh every second
- Color-coded traffic density indicators

## Configuration

To change backend URL, edit `BACKEND_URL` in `app.py`:
```python
BACKEND_URL = "http://localhost:5000/api"
```

## Display

The dashboard shows:
- Car counts per lane
- Active lane with green light
- Traffic density status (clear/light/moderate/heavy)
- Current environmental conditions
- Real-time clock
