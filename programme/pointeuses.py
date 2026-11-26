
import threading
import time
from datetime import datetime
import pymysql
from zk import ZK

from programme.base_donnee import connexion

PORT = 4370
DELAI_CONNEXION = 5          # secondes avant d'abandonner une connexion
INTERVALLE_POINTAGES = 15    # entre deux lectures des pointages
INTERVALLE_UTILISATEURS = 600
INTERVALLE_EMPREINTES = 1800
ATTENTE_MIN = 5              # première attente après un échec de connexion
ATTENTE_MAX = 120
INTERVALLE_LISTE = 60        # relecture de la table pointeuse

JOURS = ["Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche"]

_verrous = {}
_verrous_garde = threading.Lock()


def verrou(ip):
    """Verrou exclusif d'une pointeuse : à tenir pendant toute connexion."""
    with _verrous_garde:
        return _verrous.setdefault(ip, threading.Lock())


def journal(message):
    print(f"[{datetime.now():%H:%M:%S}] {message}", flush=True)


def ouvrir(ip, timeout=DELAI_CONNEXION):
    """Connexion directe, sans ping préalable : l'échec de connexion suffit
    à savoir qu'une pointeuse est injoignable, et le ping de Windows
    écrivait dans la console."""
    return ZK(ip, port=PORT, timeout=timeout, ommit_ping=True).connect()


class SurveillancePointeuse(threading.Thread):
    def __init__(self, id_pointeuse, ip, connection_factory=None):
        super().__init__(name=f"pointeuse-{id_pointeuse}", daemon=True)
        self.id_pointeuse = id_pointeuse
        self.ip = ip
        self._connection_factory = connexion if connection_factory is None else connection_factory
        self._arret = threading.Event()
        self.en_ligne = None
        self._prochaine = {"utilisateurs": 0.0, "empreintes": 0.0}

    @property
    def nom(self):
        return f"Pointeuse {self.id_pointeuse} ({self.ip})"

    def arreter(self):
        self._arret.set()

    def run(self):
        attente = ATTENTE_MIN
        while not self._arret.is_set():
            try:
                joignable = self._cycle()
            except Exception as erreur:  # le fil ne doit jamais mourir
                journal(f"{self.nom} : erreur inattendue : {erreur!r}")
                joignable = False
            if joignable:
                attente = ATTENTE_MIN
                pause = INTERVALLE_POINTAGES
            else:
                pause = attente
                attente = min(attente * 2, ATTENTE_MAX)
            self._arret.wait(pause)

    # ------------------------------------------------------------------

    def _cycle(self):
        """Un échange complet avec la pointeuse ; False si elle est injoignable."""
        with verrou(self.ip):
            try:
                conn = ouvrir(self.ip)
            except Exception as erreur:
                self._changer_etat(False, erreur)
                return False

            self._changer_etat(True)
            try:
                self._recuperer_pointages(conn)
                maintenant = time.monotonic()
                if maintenant >= self._prochaine["utilisateurs"]:
                    self._synchroniser_utilisateurs(conn)
                    self._prochaine["utilisateurs"] = maintenant + INTERVALLE_UTILISATEURS
                if maintenant >= self._prochaine["empreintes"]:
                    self._sauvegarder_empreintes(conn)
                    self._prochaine["empreintes"] = maintenant + INTERVALLE_EMPREINTES
                return True
            except pymysql.MySQLError as erreur:
                # La base est en cause, pas la pointeuse : on réessaiera.
                journal(f"{self.nom} : base de données indisponible : {erreur}")
                return True
            except Exception as erreur:
                self._changer_etat(False, erreur)
                return False
            finally:
                for action in (conn.enable_device, conn.disconnect):
                    try:
                        action()
                    except Exception:
                        pass

    def _changer_etat(self, en_ligne, cause=None):
        if en_ligne == self.en_ligne:
            return
        premier_etat = self.en_ligne is None
        self.en_ligne = en_ligne
        if en_ligne:
            journal(f"{self.nom} : {'en ligne' if premier_etat else 'reconnectée'}.")
        else:
            journal(f"{self.nom} : hors ligne ({cause}). Nouvelles tentatives automatiques.")
        try:
            with self._connection_factory() as db:
                with db.cursor() as curseur:
                    curseur.execute(
                        "UPDATE pointeuse SET EstEnligne = %s WHERE idPointeuse = %s",
                        ("En ligne" if en_ligne else "Hors ligne", self.id_pointeuse),
                    )
                db.commit()
        except pymysql.MySQLError as erreur:
            journal(f"{self.nom} : état non enregistré en base : {erreur}")

    # ------------------------------------------------------------------
    # Pointages
    # ------------------------------------------------------------------

    def _recuperer_pointages(self, conn):
        enregistrements = conn.get_attendance() or []
        if not enregistrements:
            return
        with self._connection_factory() as db:
            with db.cursor() as curseur:
                # Dernier pointage propre à CETTE pointeuse : une horloge en
                # retard sur une autre ne fait plus perdre de pointages.
                curseur.execute(
                    "SELECT MAX(date_pointage) FROM pointages WHERE idPointeuse = %s",
                    (self.id_pointeuse,),
                )
                ligne = curseur.fetchone()
                dernier = ligne[0] if ligne else None
                nouveaux = sorted(
                    (r for r in enregistrements if dernier is None or r.timestamp > dernier),
                    key=lambda r: r.timestamp,
                )
                for enregistrement in nouveaux:
                    self._enregistrer_pointage(curseur, enregistrement)
            db.commit()
        if nouveaux:
            journal(f"{self.nom} : {len(nouveaux)} nouveau(x) pointage(s).")

    def _enregistrer_pointage(self, curseur, enregistrement):
        """1er pointage du jour = arrivée, 2e = départ, suivants = départ mis à jour."""
        employe, horodatage = enregistrement.user_id, enregistrement.timestamp
        jour = JOURS[horodatage.weekday()]
        curseur.execute(
            """
            SELECT COUNT(*) FROM pointages
            WHERE IDEmploye = %s AND DATE(date_pointage) = DATE(%s) AND idPointeuse = %s
            """,
            (employe, horodatage, self.id_pointeuse),
        )
        deja = curseur.fetchone()[0]
        if deja < 2:
            curseur.execute(
                """
                INSERT IGNORE INTO pointages (IDEmploye, date_pointage, jour_pointage, idPointeuse, Status)
                VALUES (%s, %s, %s, %s, %s)
                """,
                (employe, horodatage, jour, self.id_pointeuse,
                 "Arrivée enregistrée" if deja == 0 else "Départ enregistré"),
            )
            return
        curseur.execute(
            """
            SELECT id FROM pointages
            WHERE IDEmploye = %s AND DATE(date_pointage) = DATE(%s) AND idPointeuse = %s
            ORDER BY date_pointage ASC LIMIT 1 OFFSET 1
            """,
            (employe, horodatage, self.id_pointeuse),
        )
        depart = curseur.fetchone()
        if depart:
            curseur.execute(
                "UPDATE pointages SET date_pointage = %s, Status = %s WHERE id = %s",
                (horodatage, "Départ enregistré", depart[0]),
            )

    # ------------------------------------------------------------------
    # Enseignants et empreintes
    # ------------------------------------------------------------------

    def _synchroniser_utilisateurs(self, conn):
        """Crée sur la pointeuse les enseignants de l'emploi du temps absents."""
        with self._connection_factory() as db:
            with db.cursor() as curseur:
                curseur.execute("SELECT DISTINCT professeur_code, professeur_nom FROM Programme")
                enseignants = curseur.fetchall()
        presents = {str(u.user_id) for u in conn.get_users()}
        manquants = [(code, nom) for code, nom in enseignants if code and str(code) not in presents]
        if not manquants:
            return
        conn.disable_device()
        try:
            for code, nom in manquants:
                try:
                    conn.set_user(user_id=str(code), name=str(nom or code)[:24])
                except Exception as erreur:
                    journal(f"{self.nom} : enseignant {code} non créé : {erreur}")
        finally:
            conn.enable_device()
        journal(f"{self.nom} : {len(manquants)} enseignant(s) ajouté(s).")

    def _sauvegarder_empreintes(self, conn):
        """Copie en base les empreintes de la pointeuse qui n'y sont pas encore."""
        conn.disable_device()
        try:
            utilisateurs = {u.uid: u for u in conn.get_users()}
            # Une seule requête pour toutes les empreintes, au lieu d'une par
            # utilisateur et par doigt.
            empreintes = conn.get_templates() or []
        finally:
            conn.enable_device()

        ajoutees = 0
        with self._connection_factory() as db:
            with db.cursor() as curseur:
                for empreinte in empreintes:
                    utilisateur = utilisateurs.get(empreinte.uid)
                    if utilisateur is None or not empreinte.size:
                        continue
                    curseur.execute(
                        "SELECT 1 FROM empreintes WHERE IDEmploye = %s AND IDEmpreinte = %s",
                        (utilisateur.user_id, empreinte.fid),
                    )
                    if curseur.fetchone():
                        continue
                    curseur.execute(
                        """
                        INSERT INTO empreintes (ID_DEVICE, IDEmploye, Nom, IDEmpreinte, Empreintes, IDPointeuse)
                        VALUES (%s, %s, %s, %s, %s, %s)
                        """,
                        (utilisateur.uid, utilisateur.user_id, utilisateur.name,
                         empreinte.fid, empreinte.template, self.id_pointeuse),
                    )
                    ajoutees += 1
            db.commit()
        if ajoutees:
            journal(f"{self.nom} : {ajoutees} empreinte(s) sauvegardée(s).")


class SuperviseurPointeuses:
    """Démarre un fil par pointeuse et suit les changements de la table."""

    def __init__(self, connection_factory=None, fabrique_surveillance=SurveillancePointeuse):
        self._connection_factory = connexion if connection_factory is None else connection_factory
        self._fabrique = fabrique_surveillance
        self._surveillances = {}
        self._fil = None

    def demarrer(self):
        if self._fil and self._fil.is_alive():
            return
        self._fil = threading.Thread(target=self._boucle, name="superviseur-pointeuses", daemon=True)
        self._fil.start()

    def _boucle(self):
        while True:
            try:
                self.actualiser()
            except Exception as erreur:
                journal(f"Superviseur : liste des pointeuses illisible : {erreur}")
            time.sleep(INTERVALLE_LISTE)

    def actualiser(self):
        with self._connection_factory() as db:
            with db.cursor() as curseur:
                curseur.execute("SELECT idPointeuse, AdresseIp FROM pointeuse")
                lignes = curseur.fetchall()
        voulues = {id_p: (ip or "").strip() for id_p, ip in lignes if (ip or "").strip()}

        for id_p, surveillance in list(self._surveillances.items()):
            if voulues.get(id_p) != surveillance.ip or not surveillance.is_alive():
                surveillance.arreter()
                del self._surveillances[id_p]

        for id_p, ip in voulues.items():
            if id_p not in self._surveillances:
                surveillance = self._fabrique(id_p, ip, self._connection_factory)
                self._surveillances[id_p] = surveillance
                surveillance.start()
                journal(f"Surveillance démarrée : pointeuse {id_p} ({ip}).")

    def etats(self):
        return {id_p: s.en_ligne for id_p, s in self._surveillances.items()}


superviseur = SuperviseurPointeuses()
