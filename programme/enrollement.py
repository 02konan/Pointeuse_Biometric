import os
from zk import ZK

def is_pingable(ip="192.168.1.201"):
    """Vérifie si l'adresse IP est joignable par ping"""
    response = os.system(f"ping -n 1 -w 1000 {ip}" if os.name == "nt" else f"ping -c 1 -W 1 {ip}")
    return response == 0


def enroler_utilisateur(pointeuse_ip=None, user_id=None, name=None, finger_index=None, port=4370):
    """Enrôle un utilisateur directement sur la pointeuse"""
    if not is_pingable(pointeuse_ip):
        print(f"❌ Impossible de joindre la pointeuse {pointeuse_ip}")
        return False

    zk = ZK(pointeuse_ip, port=port, timeout=20)
    conn = None
    try:
        conn = zk.connect()
        print(f"✅ Connecté à la pointeuse {pointeuse_ip}")

        # Étape 1 : enrôler l’empreinte
        print(f"👉 Placez le doigt pour enrôler {name} (ID={user_id})...")
        conn.enroll_user(uid=int(user_id), temp_id=finger_index)

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
    ip = input("Adresse IP de la pointeuse [192.168.1.201] : ") or "192.168.1.201"
    user_id = int(input("ID utilisateur : "))
    name = input("Nom de l'utilisateur : ")
    finger_index = int(input("Index du doigt (0=pouce, 1=index, etc.) [0] : ") or 0)

    enroler_utilisateur(pointeuse_ip=ip, user_id=user_id, name=name, finger_index=finger_index)
