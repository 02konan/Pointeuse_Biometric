import requests
from requests.auth import HTTPDigestAuth

# Configuration
ip_pointeuse = "192.168.1.252"
username = "admin"
password = "12345678s"

auth = HTTPDigestAuth(username, password)

url = f"http://{ip_pointeuse}/ISAPI/AccessControl/UserInfo?format=json"

try:
    response = requests.get(url, auth=auth)
    if response.status_code == 200:
        print("Données récupérées avec succès")
        print(response.text)
    else:
        print(f"Erreur: {response.status_code} - {response.text}")
except Exception as e:
    print(f"Erreur de connexion: {e}")
