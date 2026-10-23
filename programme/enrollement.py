
import os
import time
from zk import ZK
from programme.base_donnee import connexion

# Détection automatique de la première pointeuse en ligne


class FingerprintEnrollmentService:
    def __init__(self, connection_factory=None):
        self._connection_factory = (
            connexion if connection_factory is None else connection_factory
        )

    def is_pingable(self, ip):
        response = os.system(f"ping -n 1 -w 1000 {ip}" if os.name == "nt" else f"ping -c 1 -W 1 {ip}")
        return response == 0

    def get_pointeuse(self, essiae=2, attente=5):
        for _ in range(essiae):
         db = self._connection_factory()
         cursor = db.cursor()
         cursor.execute("SELECT AdresseIp FROM pointeuse")
         pointeuses = cursor.fetchall()
         cursor.close()
         db.close()
         for (ip,) in pointeuses:
            if self.is_pingable(ip):
                return ip
         print(f"⏳ Aucune pointeuse en ligne détectée. Nouvelle tentative dans {attente} secondes...")
         time.sleep(attente)
        return None

    @staticmethod
    def _find_user(conn, user_id):
        return next(
            (user for user in conn.get_users() if str(user.user_id) == str(user_id)),
            None,
        )

    def _open_connection(self):
        pointeuse=self.get_pointeuse()
        if not pointeuse:
            print("❌ Aucune pointeuse en ligne disponible pour l'enrôlement.")
            return None
        zk = ZK(pointeuse, port=4370, timeout=20)
        return zk, zk.connect(), pointeuse

    def enroler_utilisateur(self, uid=None, user_id:str="", name:str="", finger_index:int=0):
        connexion_pointeuse = None
        try:
            connexion_pointeuse = self._open_connection()
            if connexion_pointeuse is None:
                return False
            _, conn, pointeuse = connexion_pointeuse
            print(f"Connecté à la pointeuse {pointeuse}")
            utilisateur_existant = self._find_user(conn, user_id)
            if uid is None:
                uid = utilisateur_existant.uid if utilisateur_existant else conn.next_uid
            if utilisateur_existant is None:
                conn.set_user(uid=int(uid), name=str(name), user_id=str(user_id))
            print(f"Placez le doigt pour enrôler {name} (ID={user_id})...")
            if conn.enroll_user(uid=int(uid), temp_id=int(finger_index), user_id=str(user_id)):
                print(f"Utilisateur {name} enrôlé avec succès (ID={user_id}, doigt={finger_index})")
                return True
            else:
                print(f"Échec de l'enrôlement de l'utilisateur {name} (ID={user_id}, doigt={finger_index})")
                return False
        except Exception as e:
            print(f"Erreur pendant l’enrôlement : {e}")
            return False
        finally:
            if connexion_pointeuse is not None:
                connexion_pointeuse[1].disconnect()
                print("Déconnecté de la pointeuse")

    def enroler_carte(self, user_id:str="", name:str="", card_number:int=0):
        connexion_pointeuse = None
        try:
            connexion_pointeuse = self._open_connection()
            if connexion_pointeuse is None:
                return False
            _, conn, pointeuse = connexion_pointeuse
            print(f"Connecté à la pointeuse {pointeuse}")
            utilisateur_existant = self._find_user(conn, user_id)
            for user in conn.get_users():
                if user.card == int(card_number) and str(user.user_id) != str(user_id):
                    print(f"Cette carte est déjà attribuée à l'utilisateur {user.user_id}.")
                    return False
            uid = utilisateur_existant.uid if utilisateur_existant else conn.next_uid
            succes = conn.set_user(
                uid=int(uid),
                name=str(name),
                user_id=str(user_id),
                card=int(card_number),
            )
            if succes:
                print(f"Carte enrôlée pour {name} (ID={user_id}).")
            return bool(succes)
        except Exception as e:
            print(f"Erreur pendant l’enrôlement de la carte : {e}")
            return False
        finally:
            if connexion_pointeuse is not None:
                connexion_pointeuse[1].disconnect()
                print("Déconnecté de la pointeuse")


_service = FingerprintEnrollmentService(connection_factory=connexion)
is_pingable = _service.is_pingable
get_pointeuse = _service.get_pointeuse
enroler_utilisateur = _service.enroler_utilisateur
enroler_carte = _service.enroler_carte
