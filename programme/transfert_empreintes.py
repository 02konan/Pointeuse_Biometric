import pymysql
import time
import os
from zk import ZK, const
from programme.base_donnee import connexion


class FingerprintTransferService:
    def __init__(self, connection_factory=None):
        self._connection_factory = (
            connexion if connection_factory is None else connection_factory
        )

    def is_pingable(self, ip):
        response = os.system(f"ping -n 1 -w 1000 {ip}" if os.name == "nt" else f"ping -c 1 -W 1 {ip}")
        return response == 0

    def get_pointeuses_template(self, pointeuse_id):
        with self._connection_factory() as db:
            cursor = db.cursor()
            cursor.execute("SELECT IDEmploye, IDEmpreinte, Empreintes, Nom FROM empreintes WHERE IDPointeuse=%s", (pointeuse_id,))
            return cursor.fetchall()

    def transfert_empreintes(self, id_pointeuse_source=1, ip_pointeuse_cible="192.168.1.202"):
        """
        Synchronise les empreintes de la pointeuse source (en base) vers la pointeuse cible (matériel).
        """
        empreintes = self.get_pointeuses_template(id_pointeuse_source)
        if not empreintes:
            print(f"Aucune empreinte trouvée pour la pointeuse source ID={id_pointeuse_source}")
            return

        if not self.is_pingable(ip_pointeuse_cible):
            print(f"[ERREUR] Impossible de joindre la pointeuse cible {ip_pointeuse_cible}.")
            return

        zk = ZK(ip_pointeuse_cible, port=4370, timeout=20)
        try:
            print(f"[INFO] Connexion à la pointeuse cible {ip_pointeuse_cible}...")
            conn = zk.connect()
            conn.disable_device()
            print(f"[INFO] Connexion réussie à la pointeuse {ip_pointeuse_cible}.")

            users_cible = {u.uid: u for u in conn.get_users()}

            for emp in empreintes:
                id_employe, id_empreinte, empreinte_data, nom = emp
                try:
                    uid = int(id_employe)
                except Exception:
                    print(f"[ERREUR] L'ID employé '{id_employe}' n'est pas un entier. Empreinte ignorée.")
                    continue
                finger_id = int(id_empreinte)
                # Vérifier si l'utilisateur existe sur la cible, sinon le créer
                if uid not in users_cible:
                    print(f"[INFO] Création de l'utilisateur {nom} (UID={uid}) sur la pointeuse cible...")
                    conn.set_user(uid=uid, name=nom, user_id=str(id_employe))
                # Vérifier si l'empreinte existe déjà sur la cible pour ce finger_id
                template_cible = conn.get_user_template(uid, finger_id)
                if template_cible and hasattr(template_cible, 'template') and template_cible.template == empreinte_data:
                    print(f"[OK] Empreinte déjà présente pour l'utilisateur {uid} doigt {finger_id}.")
                else:
                    # Ajout ou mise à jour de l'empreinte
                    print(f"[SYNC] Ajout/MàJ empreinte utilisateur {uid} doigt {finger_id} sur la cible...")
                    conn.set_user_template(uid, finger_id, empreinte_data)

            conn.enable_device()
            conn.disconnect()
            print("[INFO] Synchronisation terminée.")
        except Exception as e:
            print(f"[ERREUR] Échec de synchronisation avec la pointeuse cible {ip_pointeuse_cible} : {e}")


_service = FingerprintTransferService(connection_factory=connexion)
is_pingable = _service.is_pingable
get_pointeuses_template = _service.get_pointeuses_template
transfert_empreintes = _service.transfert_empreintes
