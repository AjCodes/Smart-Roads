from flask import Flask, render_template, jsonify
import requests
from datetime import datetime

app = Flask(__name__)

# Backend API URL - Adjust if needed
BACKEND_URL = "http://localhost:5000/api"

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/dashboard-data')
def get_dashboard_data():
    try:
        # Fetch latest sensor data
        sensor_response = requests.get(f"{BACKEND_URL}/sensor-data/latest")
        sensor_data = sensor_response.json().get('data', {})

        # Fetch latest decision
        decision_response = requests.get(f"{BACKEND_URL}/decision/latest")
        decision_data = decision_response.json().get('decision', {})

        # Fetch history (optional, for charts if added later)
        # history_response = requests.get(f"{BACKEND_URL}/decisions?limit=10")
        
        return jsonify({
            'success': True,
            'sensor': sensor_data,
            'decision': decision_data,
            'timestamp': datetime.now().strftime("%H:%M:%S")
        })
    except Exception as e:
        return jsonify({
            'success': False,
            'error': str(e)
        })

if __name__ == '__main__':
    app.run(debug=True, port=5001)
