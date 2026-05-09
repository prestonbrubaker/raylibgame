import requests
import time
import json

URL = "http://willwill.immenseaccumulationonline.online:8080/update"

print("=== HTTP TUNNEL DEBUG ===")
print(f"Target: {URL}\n")

player_id = 1
x, y, z = 0.0, 0.0, 0.0

for i in range(10):
    # Fake player movement
    x += 1.5
    z += 0.8
    
    data = {
        "id": player_id,
        "x": round(x, 2),
        "y": round(y, 2),
        "z": round(z, 2)
    }
    
    try:
        response = requests.post(URL, json=data, timeout=5)
        print(f"Sent: {data}")
        print(f"Status: {response.status_code}")
        print(f"Response: {response.text[:200]}")
        print("-" * 40)
    except Exception as e:
        print(f"Error: {e}")
    
    time.sleep(1)

print("\n=== Test Complete ===")