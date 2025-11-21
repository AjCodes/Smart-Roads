import requests
import time
import random
import json

# Backend URL (Localhost)
URL = "http://localhost:5000/api/sensor-data"

def simulate_traffic():
    print(f"🚀 Starting ESP32 Simulation... Sending data to {URL}")
    print("Press Ctrl+C to stop.\n")

    try:
        while True:
            # Generate random distances (simulating traffic)
            # Lower distance = More traffic
            data = {
                "lane1": random.randint(20, 400),
                "lane2": random.randint(20, 400),
                "lane3": random.randint(20, 400),
                "lane4": random.randint(20, 400)
            }

            print(f"📤 Sending: {data}")

            try:
                response = requests.post(URL, json=data)
                
                if response.status_code == 201:
                    result = response.json()
                    decision = result.get('decision', {})
                    print(f"✅ Received Decision: Lane={decision.get('activeLane')}, Duration={decision.get('duration')}s")
                    print(f"   Reason: {decision.get('reason')}")
                else:
                    print(f"❌ Error: {response.status_code} - {response.text}")

            except requests.exceptions.ConnectionError:
                print("❌ Connection Error: Is the Backend Server running?")

            print("-" * 50)
            time.sleep(5) # Send every 5 seconds

    except KeyboardInterrupt:
        print("\n🛑 Simulation stopped.")

if __name__ == "__main__":
    simulate_traffic()
