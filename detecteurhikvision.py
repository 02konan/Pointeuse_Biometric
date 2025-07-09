import requests
from requests.auth import HTTPBasicAuth
import xml.etree.ElementTree as ET

ip = "192.168.1.252"
user = "admin"
password = "12345678s"
url = f"http://{ip}/ISAPI/AccessControl/Fingerprint/Search"

payload = """
<AcsEventCond>
    <searchID>1</searchID>
    <maxResults>30</maxResults>
</AcsEventCond>
"""

headers = {
    "Content-Type": "application/xml"
}

response = requests.post(url, data=payload, headers=headers, auth=HTTPBasicAuth(user, password))

# Afficher la réponse brute (XML)
print(response.text)

# Exemple de parsing XML : afficher tous les événements
root = ET.fromstring(response.text)
for event in root.findall(".//AcsEvent"):
    time = event.find("time").text if event.find("time") is not None else ""
    cardNo = event.find("cardNo").text if event.find("cardNo") is not None else ""
    employeeNoString = event.find("employeeNoString").text if event.find("employeeNoString") is not None else ""
    entranceName = event.find("entranceName").text if event.find("entranceName") is not None else ""
    print(f"Date/Heure : {time} | Carte : {cardNo} | Employé : {employeeNoString} | Entrée : {entranceName}")
