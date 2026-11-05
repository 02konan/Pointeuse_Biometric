import pymysql
import time
import os
from zk import ZK, const
from programme.base_donnee import connexion


class DeviceUserSyncService:
    def __init__(self, connection_factory=None):
        self._connection_factory = (
            connexion if connection_factory is None else connection_factory
        )

    def is_pingable(self, ip):
        response = os.system(f"ping -n 1 -w 1000 {ip}" if os.name == "nt" else f"ping -c 1 -W 1 {ip}")
        return response == 0

    def get_pointeuses(self):
        db = self._connection_factory()
        cursor = db.cursor()
        cursor.execute("SELECT Adresseip, idPointeuse FROM pointeuse")
        result = cursor.fetchall()
        db.close()
        return result

    def get_employes(self):
        try:
            with self._connection_factory() as db:
                cursor = db.cursor()
                cursor.execute("SELECT DISTINCT(professeur_code), professeur_nom FROM Programme")
                return cursor.fetchall()
        except pymysql.MySQLError as e:
            print("❌ Erreur lors de la récupération des employés :", e)
            return []

    def insertion_(self):
        while True:
            pointeuse=self.get_pointeuses()
            for pointeuse in pointeuse:
                pointeuse_ip = pointeuse[0]
                if not self.is_pingable(pointeuse_ip):
                    print(f"[ERREUR] Impossible de joindre la pointeuse {pointeuse_ip}. Nouvel essai dans 5 secondes...")
                    continue
                zk = ZK(pointeuse_ip, port=4370, timeout=20)
            try:
                print(f"[INFO] Connexion à la pointeuse {pointeuse_ip}...")
                conn = zk.connect()
                conn.disable_device()
                print(f"[INFO] Connexion réussie à la pointeuse {pointeuse_ip}.")

                employes = self.get_employes()
                for emp in employes:
                    code, nom = emp
                    try:
                        user_exists = any(user.user_id == str(code) for user in conn.get_users())
                        if user_exists:
                            print(f"[INFO] L'employé {nom} (Code: {code}) existe déjà sur la pointeuse.")
                        else:
                            conn.set_user(user_id=str(code), name=str(nom))
                            print(f"[INFO] Employé {nom} (Code: {code}) inséré avec succès.")
                    except Exception as e:
                        print(f"[ERREUR] Échec de l'insertion pour {nom} (Code: {code}) : {e}")

                conn.enable_device()

            except Exception as e:
                print(f"[ERREUR] Échec de connexion à la pointeuse {pointeuse_ip} : {e}")
                zk.disconnect()
            finally:
             time.sleep(5)


_service = DeviceUserSyncService(connection_factory=connexion)
is_pingable = _service.is_pingable
get_pointeuses = _service.get_pointeuses
get_employes = _service.get_employes
insertion_ = _service.insertion_
