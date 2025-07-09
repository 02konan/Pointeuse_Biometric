import requests
from requests.auth import HTTPDigestAuth
import json

ip = "192.168.1.252"  # Adresse de ta pointeuse
user = "admin"
password = "12345678s"

url = f"http://{ip}/ISAPI/AccessControl/UserInfo/Search"

payload = {
    "AcsUserCond": {
        "searchID": 1,
        "searchResultPosition": 0,
        "maxResults": 10,
        "major": 5,
        "minor": 75
    }
}

headers = {
    "Content-Type": "application/json"
}

response = requests.post(
    url,
    headers=headers,
    auth=HTTPDigestAuth(user, password),
    data=json.dumps(payload)
)

if response.status_code == 200:
    print("Réponse brute :")
    print(response.text)
else:
    print(f"Erreur {response.status_code} : {response.text}")

