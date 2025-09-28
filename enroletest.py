
import os
import time
from zk import ZK
from programme.base_donnee import connexion

# Détection automatique de la première pointeuse en ligne
def is_pingable(ip):
    response = os.system(f"ping -n 1 -w 1000 {ip}" if os.name == "nt" else f"ping -c 1 -W 1 {ip}")
    return response == 0

def get_pointeuse(essiae=2, attente=5):
    for _ in range(essiae):
     db = connexion()
     cursor = db.cursor()
     cursor.execute("SELECT AdresseIp FROM pointeuse")
     pointeuses = cursor.fetchall()
     cursor.close()
     db.close()
     for (ip,) in pointeuses:
        if is_pingable(ip):
            return ip
     print(f"⏳ Aucune pointeuse en ligne détectée. Nouvelle tentative dans {attente} secondes...")
     time.sleep(attente)   
    return None

def enrolement(user_id:str=None,name:str=None,finger_index:int=None):
    pointeuse=get_pointeuse()
    if not pointeuse:
        print("❌ Aucune pointeuse en ligne disponible pour l'enrôlement.")
        return False
    zk = ZK(pointeuse, port=4370, timeout=20)
    conn=zk.connect()
    print(f"Connecté à la pointeuse {pointeuse}")
    try:
        print(f"Placez le doigt pour enrôler {name} (ID={user_id})... (10 secondes)")
        time.sleep(10)
        conn.enroll_user(user_id=str(user_id),temp_id=finger_index)
        print(f"Utilisateur {name} enrôlé avec succès (ID={user_id}, doigt={finger_index})")
        return True
    except Exception as e:
        print(f"Erreur pendant l’enrôlement : {e}")
        return False
    finally:
        conn.disconnect()
        print("Déconnecté de la pointeuse")