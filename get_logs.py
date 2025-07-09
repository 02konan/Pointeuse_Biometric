import requests
from requests.auth import HTTPBasicAuth

ip = "192.168.1.252"  # Adresse de ta pointeuse
user = "admin"
password = "12345678s"
url = f"http://{ip}/ISAPI/AccessControl/AcsEvent"

payload = """
<AcsEventCond>
    <searchID>1</searchID>
    <maxResults>30</maxResults>
    <major>5</major>
    <minor>75</minor>
</AcsEventCond>
"""

headers = {
    "Content-Type": "application/xml"
}

response = requests.post(url, data=payload, headers=headers, auth=HTTPBasicAuth(user, password))
print(response.text)