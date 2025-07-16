import requests
from requests.auth import HTTPBasicAuth

url = "http://192.168.1.252/ISAPI/AccessControl/UserInfo/Search"

headers = {
    "Content-Type": "application/xml"
}

data = """
<UserInfoSearchCond>
    <searchID>1</searchID>
    <maxResults>50</maxResults>
    <searchResultPosition>0</searchResultPosition>
</UserInfoSearchCond>
"""

response = requests.post(url, data=data, headers=headers, auth=HTTPBasicAuth('admin', '@IFSM2025'))

print(response.status_code)
print(response.text)
