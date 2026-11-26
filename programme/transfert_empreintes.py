import pymysql
import time
from programme.base_donnee import connexion
from programme.pointeuses import ouvrir, verrou


class FingerprintTransferService:
    def __init__(self, connection_factory=None):
        self._connection_factory = (
            connexion if connection_factory is None else connection_factory
        )

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

        # Sous le verrou de la pointeuse cible : sa surveillance attend la fin
        # du transfert au lieu d'ouvrir une connexion en même temps.
        with verrou(ip_pointeuse_cible):
            try:
                print(f"[INFO] Connexion à la pointeuse cible {ip_pointeuse_cible}...")
                conn = ouvrir(ip_pointeuse_cible, timeout=20)
            except Exception as e:
                print(f"[ERREUR] Impossible de joindre la pointeuse cible {ip_pointeuse_cible} : {e}")
                return
            try:
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

                print("[INFO] Synchronisation terminée.")
            except Exception as e:
                print(f"[ERREUR] Échec de synchronisation avec la pointeuse cible {ip_pointeuse_cible} : {e}")
            finally:
                for action in (conn.enable_device, conn.disconnect):
                    try:
                        action()
                    except Exception:
                        pass


_service = FingerprintTransferService(connection_factory=connexion)
get_pointeuses_template = _service.get_pointeuses_template
transfert_empreintes = _service.transfert_empreintes
