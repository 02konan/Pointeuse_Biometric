"""Emploi du temps : lecture de la table Programme et import depuis un fichier.

Le fichier (CSV ou Excel .xlsx) contient une ligne par cours. Les en-têtes
sont reconnus sans tenir compte des majuscules, des accents ni des espaces :

    matricule ; enseignant ; jour ; matiere ; heure_debut ; heure_fin ; section ; type

L'import est « tout ou rien » : si une seule ligne est invalide, rien n'est
écrit et toutes les erreurs sont rapportées avec leur numéro de ligne.

Un cours est identifié par (matricule, jour, heure de début) : réimporter
le même fichier met les cours à jour au lieu de les dupliquer. Aucune ligne
n'est supprimée, car pointage_programe référence Programme.IDProgramme.
"""

import csv
import io
import unicodedata
from datetime import datetime, time, timedelta

import pymysql

from programme.base_donnee import connexion

JOURS = ["Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche"]
TYPES = ("definitif", "modulaire")
TAILLE_MAX = 2 * 1024 * 1024

# Libellés acceptés pour chaque colonne (après normalisation).
COLONNES = {
    "matricule": ("matricule", "code", "professeur_code", "code_professeur"),
    "enseignant": ("enseignant", "professeur", "nom", "professeur_nom", "nom_enseignant"),
    "jour": ("jour",),
    "matiere": ("matiere", "cours", "module", "nom_matiere"),
    "heure_debut": ("heure_debut", "debut", "heure_arrivee", "arrivee"),
    "heure_fin": ("heure_fin", "fin", "heure_depart", "depart"),
    "section": ("section", "classe"),
    "type": ("type", "type_edt"),
    "professeur_id": ("professeur_id", "id_professeur"),
    "statut": ("statut", "status"),
}
OBLIGATOIRES = ("matricule", "enseignant", "jour", "heure_debut", "heure_fin")

MODELE_CSV = (
    "matricule;enseignant;jour;matiere;heure_debut;heure_fin;section;type\n"
    "PROF001;KOUASSI Jean;Lundi;Comptabilité;08:00;10:00;CDY;definitif\n"
    "PROF001;KOUASSI Jean;Mercredi;Gestion;10:15;12:15;CDY;modulaire\n"
)


class ErreurImport(Exception):
    """Fichier illisible ou contenu invalide ; `erreurs` liste le détail."""

    def __init__(self, message, erreurs=None):
        super().__init__(message)
        self.erreurs = erreurs or []


def _normaliser(texte):
    texte = unicodedata.normalize("NFKD", str(texte or "")).encode("ascii", "ignore").decode()
    return "_".join(texte.strip().lower().replace("-", " ").split())


def _texte(valeur):
    return "" if valeur is None else str(valeur).strip()


class EmploiDuTempsService:
    def __init__(self, connection_factory=None):
        self._connection_factory = connexion if connection_factory is None else connection_factory
        self._nom_table = None

    def _table(self, curseur):
        """Nom réel de la table des cours : « Programme » ou « programme ».

        Sous Linux, MySQL distingue les majuscules dans les noms de table ;
        on retrouve donc la table quelle que soit sa casse.
        """
        if self._nom_table is None:
            curseur.execute(
                "SELECT table_name FROM information_schema.tables "
                "WHERE table_schema = DATABASE() AND LOWER(table_name) = 'programme' "
                "LIMIT 1"
            )
            ligne = curseur.fetchone()
            if not ligne:
                raise ErreurImport("La table « programme » est introuvable dans la base de données.")
            self._nom_table = ligne[0]
        return f"`{self._nom_table}`"

    # ------------------------------------------------------------------
    # Lecture du fichier
    # ------------------------------------------------------------------

    def lire_fichier(self, nom, contenu):
        """Retourne la liste des lignes (dict en-tête -> valeur) du fichier."""
        if not contenu:
            raise ErreurImport("Le fichier est vide.")
        if len(contenu) > TAILLE_MAX:
            raise ErreurImport("Le fichier dépasse 2 Mo.")

        extension = (nom or "").rsplit(".", 1)[-1].lower()
        if extension == "xlsx":
            tableau = self._lire_xlsx(contenu)
        elif extension == "csv":
            tableau = self._lire_csv(contenu)
        else:
            raise ErreurImport("Format non pris en charge : utilisez un fichier .csv ou .xlsx.")

        tableau = [ligne for ligne in tableau if any(_texte(v) for v in ligne)]
        if len(tableau) < 2:
            raise ErreurImport("Le fichier ne contient aucun cours.")

        entetes = [self._colonne(e) for e in tableau[0]]
        manquantes = [c for c in OBLIGATOIRES if c not in entetes]
        if manquantes:
            raise ErreurImport(
                "Colonnes manquantes : " + ", ".join(manquantes)
                + ". Téléchargez le modèle pour voir le format attendu."
            )

        lignes = []
        for valeurs in tableau[1:]:
            lignes.append({
                entete: valeur
                for entete, valeur in zip(entetes, valeurs)
                if entete is not None
            })
        return lignes

    @staticmethod
    def _colonne(entete):
        cle = _normaliser(entete)
        for colonne, synonymes in COLONNES.items():
            if cle in synonymes:
                return colonne
        return None

    @staticmethod
    def _lire_csv(contenu):
        for encodage in ("utf-8-sig", "cp1252"):
            try:
                texte = contenu.decode(encodage)
                break
            except UnicodeDecodeError:
                continue
        try:
            delimiteur = csv.Sniffer().sniff(texte.splitlines()[0], delimiters=";,\t").delimiter
        except csv.Error:
            delimiteur = ";"  # séparateur d'Excel en français
        return list(csv.reader(io.StringIO(texte), delimiter=delimiteur))

    @staticmethod
    def _lire_xlsx(contenu):
        try:
            from openpyxl import load_workbook
            classeur = load_workbook(io.BytesIO(contenu), read_only=True, data_only=True)
        except Exception as erreur:
            raise ErreurImport(f"Fichier Excel illisible : {erreur}")
        feuille = classeur.worksheets[0]
        return [list(ligne) for ligne in feuille.iter_rows(values_only=True)]

    # ------------------------------------------------------------------
    # Validation
    # ------------------------------------------------------------------

    def valider(self, lignes, section_par_defaut=""):
        """Convertit les lignes en cours prêts à écrire, ou lève ErreurImport."""
        cours, erreurs, vus = [], [], {}
        for numero, ligne in enumerate(lignes, start=2):  # ligne 1 = en-têtes
            problemes = []
            matricule = _texte(ligne.get("matricule"))
            enseignant = _texte(ligne.get("enseignant"))
            section = _texte(ligne.get("section")) or section_par_defaut.strip()
            jour = self._jour(ligne.get("jour"))
            debut = self._heure(ligne.get("heure_debut"))
            fin = self._heure(ligne.get("heure_fin"))
            type_edt = _normaliser(ligne.get("type")) or "definitif"

            if not matricule:
                problemes.append("matricule absent")
            if not enseignant:
                problemes.append("nom de l'enseignant absent")
            if not section:
                problemes.append("section absente")
            if jour is None:
                problemes.append(f"jour « {_texte(ligne.get('jour'))} » inconnu (Lundi … Dimanche)")
            if debut is None:
                problemes.append(f"heure de début « {_texte(ligne.get('heure_debut'))} » invalide (HH:MM)")
            if fin is None:
                problemes.append(f"heure de fin « {_texte(ligne.get('heure_fin'))} » invalide (HH:MM)")
            if debut and fin and fin <= debut:
                problemes.append("l'heure de fin doit suivre l'heure de début")
            if type_edt not in TYPES:
                problemes.append(f"type « {_texte(ligne.get('type'))} » inconnu (definitif ou modulaire)")

            if not problemes:
                cle = (matricule, jour, debut)
                if cle in vus:
                    problemes.append(f"même cours que la ligne {vus[cle]}")
                vus[cle] = numero

            if problemes:
                erreurs.append(f"Ligne {numero} : " + ", ".join(problemes) + ".")
                continue

            duree = datetime.combine(datetime.min, fin) - datetime.combine(datetime.min, debut)
            cours.append({
                "professeur_code": matricule,
                "professeur_nom": enseignant,
                "jour": jour,
                "matiere": _texte(ligne.get("matiere")) or None,
                "heure_arrivee": debut.strftime("%H:%M:%S"),
                "heure_depart": fin.strftime("%H:%M:%S"),
                "duree_cours": self._duree(duree),
                "section": section,
                "type_edt": type_edt,
                "professeur_id": _texte(ligne.get("professeur_id")) or None,
                "statut": _texte(ligne.get("statut")) or None,
            })

        if erreurs:
            raise ErreurImport(f"{len(erreurs)} ligne(s) invalide(s) : aucun cours importé.", erreurs)
        return cours

    @staticmethod
    def _jour(valeur):
        cle = _normaliser(valeur)
        if not cle:
            return None
        for jour in JOURS:
            nom = _normaliser(jour)
            if cle == nom or (len(cle) >= 3 and nom.startswith(cle)):
                return jour
        return None

    @staticmethod
    def _heure(valeur):
        if isinstance(valeur, datetime):
            return valeur.time().replace(second=0, microsecond=0)
        if isinstance(valeur, time):
            return valeur.replace(second=0, microsecond=0)
        if isinstance(valeur, timedelta):
            valeur = (datetime.min + valeur).time()
            return valeur.replace(second=0, microsecond=0)
        if isinstance(valeur, float) and 0 <= valeur < 1:
            # Heure Excel stockée en fraction de journée.
            minutes = round(valeur * 24 * 60)
            return time(minutes // 60, minutes % 60)
        texte = _texte(valeur).lower().replace("h", ":").replace(".", ":")
        if texte.endswith(":"):
            texte += "00"
        for format_ in ("%H:%M", "%H:%M:%S", "%H"):
            try:
                return datetime.strptime(texte, format_).time()
            except ValueError:
                continue
        return None

    @staticmethod
    def _duree(delta):
        minutes = int(delta.total_seconds() // 60)
        return f"{minutes // 60:02d}:{minutes % 60:02d}:00"

    # ------------------------------------------------------------------
    # Écriture
    # ------------------------------------------------------------------

    def importer(self, cours):
        """Écrit les cours dans une seule transaction ; retourne les compteurs."""
        ajoutes = mis_a_jour = 0
        try:
            with self._connection_factory() as conn:
                with conn.cursor() as curseur:
                    table = self._table(curseur)
                    for c in cours:
                        curseur.execute(
                            f"""
                            SELECT IDProgramme, professeur_id FROM {table}
                            WHERE professeur_code = %s AND jour = %s AND heure_arrivee = %s
                            LIMIT 1
                            """,
                            (c["professeur_code"], c["jour"], c["heure_arrivee"]),
                        )
                        existant = curseur.fetchone()
                        if existant:
                            curseur.execute(
                                f"""
                                UPDATE {table}
                                SET professeur_nom = %s, Matiere = %s, heure_depart = %s,
                                    duree_cours = %s, section = %s, type_edt = %s,
                                    status = COALESCE(%s, status)
                                WHERE IDProgramme = %s
                                """,
                                (c["professeur_nom"], c["matiere"], c["heure_depart"],
                                 c["duree_cours"], c["section"], c["type_edt"],
                                 c["statut"], existant[0]),
                            )
                            mis_a_jour += 1
                            continue

                        # Nouveau cours : on reprend l'identifiant EduFlow de
                        # l'enseignant s'il a déjà des cours, à défaut celui du fichier.
                        professeur_id = c["professeur_id"]
                        if professeur_id is None:
                            curseur.execute(
                                f"SELECT professeur_id FROM {table} WHERE professeur_code = %s "
                                "AND professeur_id IS NOT NULL LIMIT 1",
                                (c["professeur_code"],),
                            )
                            ligne = curseur.fetchone()
                            professeur_id = ligne[0] if ligne else None
                        curseur.execute(
                            f"""
                            INSERT INTO {table} (professeur_id, professeur_code, professeur_nom,
                                jour, Matiere, heure_arrivee, heure_depart, duree_cours,
                                section, type_edt, status)
                            VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
                            """,
                            (professeur_id, c["professeur_code"], c["professeur_nom"],
                             c["jour"], c["matiere"], c["heure_arrivee"], c["heure_depart"],
                             c["duree_cours"], c["section"], c["type_edt"], c["statut"]),
                        )
                        ajoutes += 1
                conn.commit()
        except pymysql.MySQLError as erreur:
            raise ErreurImport(
                f"La base de données a refusé l'import, rien n'a été enregistré : {erreur}"
            )
        return {"ajoutes": ajoutes, "mis_a_jour": mis_a_jour}

    def lister(self):
        """Tous les cours ; lève ErreurImport si la base est illisible."""
        try:
            with self._connection_factory() as conn:
                with conn.cursor() as curseur:
                    table = self._table(curseur)
                    curseur.execute(
                        f"""
                        SELECT professeur_code, professeur_nom, jour, Matiere,
                               heure_arrivee, heure_depart, duree_cours, section, type_edt
                        FROM {table}
                        ORDER BY FIELD(jour, 'Lundi', 'Mardi', 'Mercredi', 'Jeudi',
                                       'Vendredi', 'Samedi', 'Dimanche'),
                                 heure_arrivee, professeur_nom
                        """
                    )
                    lignes = curseur.fetchall()
        except pymysql.MySQLError as erreur:
            print(f"[ERREUR] Lecture de l'emploi du temps : {erreur}")
            raise ErreurImport(f"Lecture de l'emploi du temps impossible : {erreur}")
        return [
            {
                "matricule": l[0],
                "enseignant": l[1],
                "jour": l[2],
                "matiere": l[3],
                "debut": self._affichage(l[4]),
                "fin": self._affichage(l[5]),
                "duree": self._affichage(l[6]),
                "section": l[7],
                "type": l[8],
            }
            for l in lignes
        ]

    @staticmethod
    def _affichage(valeur):
        """« 8:00:00 », timedelta ou time → « 08:00 »."""
        if valeur is None:
            return ""
        if isinstance(valeur, timedelta):
            minutes = int(valeur.total_seconds() // 60)
            return f"{minutes // 60:02d}:{minutes % 60:02d}"
        if isinstance(valeur, time):
            return valeur.strftime("%H:%M")
        heure = EmploiDuTempsService._heure(valeur)
        return heure.strftime("%H:%M") if heure else str(valeur)


_service = EmploiDuTempsService()
