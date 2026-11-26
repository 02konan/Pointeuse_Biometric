
from programme.base_donnee import connexion
from programme.pointeuses import journal, ouvrir, verrou

ATTENTE_VERROU = 30  


class FingerprintEnrollmentService:
    def __init__(self, connection_factory=None):
        self._connection_factory = (
            connexion if connection_factory is None else connection_factory
        )
        self._verrous_tenus = {}

    def pointeuses_candidates(self):
        with self._connection_factory() as db:
            with db.cursor() as cursor:
                cursor.execute(
                    "SELECT AdresseIp FROM pointeuse "
                    "ORDER BY (EstEnligne = 'En ligne') DESC, idPointeuse"
                )
                return [ip.strip() for (ip,) in cursor.fetchall() if ip and ip.strip()]

    @staticmethod
    def _find_user(conn, user_id):
        return next(
            (user for user in conn.get_users() if str(user.user_id) == str(user_id)),
            None,
        )

    def _open_connection(self):
       
        for ip in self.pointeuses_candidates():
            acces = verrou(ip)
            if not acces.acquire(timeout=ATTENTE_VERROU):
                journal(f"Enrôlement : pointeuse {ip} occupée, essai de la suivante.")
                continue
            try:
                conn = ouvrir(ip, timeout=20)
            except Exception as erreur:
                acces.release()
                journal(f"Enrôlement : pointeuse {ip} injoignable ({erreur}).")
                continue
            self._verrous_tenus[id(conn)] = acces
            return None, conn, ip
        print("❌ Aucune pointeuse en ligne disponible pour l'enrôlement.")
        return None

    def _fermer(self, connexion_pointeuse):
        conn = connexion_pointeuse[1]
        try:
            conn.disconnect()
            print("Déconnecté de la pointeuse")
        finally:
            acces = self._verrous_tenus.pop(id(conn), None)
            if acces is not None:
                acces.release()

    @staticmethod
    def _empreinte(conn, uid, finger_index):
        """Modèle d'empreinte présent sur la pointeuse pour ce doigt, ou None."""
        try:
            modele = conn.get_user_template(uid=int(uid), temp_id=int(finger_index))
            if modele is None or not modele.size:
                return None
            return bytes(modele.template)
        except Exception:
            return None

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
            avant = self._empreinte(conn, uid, finger_index)
            print(f"Placez le doigt pour enrôler {name} (ID={user_id})...")
            signale = conn.enroll_user(uid=int(uid), temp_id=int(finger_index), user_id=str(user_id))
            # pyzk renvoie False avec certains firmwares alors que l'empreinte
            # est enregistrée : on vérifie sur la pointeuse qu'elle a changé.
            apres = self._empreinte(conn, uid, finger_index)
            if signale or (apres is not None and apres != avant):
                print(f"Utilisateur {name} enrôlé avec succès (ID={user_id}, doigt={finger_index})")
                return True
            print(f"Échec de l'enrôlement de l'utilisateur {name} (ID={user_id}, doigt={finger_index})")
            return False
        except Exception as e:
            print(f"Erreur pendant l’enrôlement : {e}")
            return False
        finally:
            if connexion_pointeuse is not None:
                self._fermer(connexion_pointeuse)

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
                self._fermer(connexion_pointeuse)


_service = FingerprintEnrollmentService(connection_factory=connexion)
enroler_utilisateur = _service.enroler_utilisateur
enroler_carte = _service.enroler_carte
