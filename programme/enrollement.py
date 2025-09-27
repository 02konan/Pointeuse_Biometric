
import os
import time
from zk import ZK
from programme.base_donnee import connexion

# Détection automatique de la première pointeuse en ligne
def is_pingable(ip):
    response = os.system(f"ping -n 1 -w 1000 {ip}" if os.name == "nt" else f"ping -c 1 -W 1 {ip}")
    return response == 0

def get_first_online_pointeuse(essiae=2, attente=5):
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


def enroler_utilisateur(user_id=None, name=None, finger_index=None):
    """Enrôle un utilisateur sur la première pointeuse détectée en ligne"""
    pointeuse_ip = get_first_online_pointeuse()
    if not pointeuse_ip:
        print("❌ Aucune pointeuse en ligne détectée !")
        return False

    zk = ZK(pointeuse_ip, port=4370, timeout=20)
    conn = None
    try:
        conn = zk.connect()
        print(f"✅ Connecté à la pointeuse {pointeuse_ip}")
        users = conn.get_users()
        last_uid = max((int(u.uid) for u in users)) if users else 1
        new_uid = last_uid + 1

        # Étape 1 : enrôler l’empreinte
        print(f"👉 Placez le doigt pour enrôler {name} (ID={user_id})... (30 secondes)")
        time.sleep(30)
        conn.enroll_user(uid=int(new_uid), temp_id=finger_index)

        # Étape 2 : associer le nom et ID
        conn.set_user(name=name, user_id=str(user_id))

        print(f"👤 Utilisateur {name} enrôlé avec succès (ID={user_id}, doigt={finger_index})")
        return True

    except Exception as e:
        print(f"⚠️ Erreur pendant l’enrôlement : {e}")
        return False

    finally:
        if conn:
            conn.disconnect()
            print("🔌 Déconnecté de la pointeuse")

if __name__ == "__main__":
    enroler_utilisateur(user_id=1234, name="Test User", finger_index=0)