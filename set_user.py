import requests
from requests.auth import HTTPBasicAuth

url = "http://192.168.1.252/ISAPI/System/deviceInfo"

response = requests.get(url, auth=HTTPBasicAuth("admin", "12345678s"))

print(response.status_code)
print(response.text)
