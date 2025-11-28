# 📊 Smart Roads Dashboard

A Python-based dashboard to visualize real-time traffic data and AI decisions.

---

## ⚙️ Setup

**1. Install Dependencies**
```bash
pip install -r requirements.txt
```
*Dependencies: `flask`, `requests`*

**2. Run Dashboard**
```bash
python app.py
```

✅ Dashboard starts at `http://localhost:5001`

---

## 🖥️ Features

- **Real-time Monitoring**: Fetches latest sensor readings from the backend.
- **AI Decisions**: Displays the current traffic light status decided by the AI.
- **Auto-Refresh**: Polls the backend every few seconds for updates.

---

## 🔧 Configuration

The dashboard connects to the backend at `http://localhost:5000`.
To change this, edit `BACKEND_URL` in `app.py`.
