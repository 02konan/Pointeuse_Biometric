"""Accès aux données pour l'application mobile.

Ce module ne contient que des requêtes : la couche HTTP (routes, jetons,
codes d'erreur) vit dans App/routes.py. Les requêtes reprennent les
jointures déjà utilisées par l'application web afin de coller au schéma
existant (pointages / empreintes / employe / Programme).
"""

from datetime import date, datetime, timedelta

import pymysql

from programme.base_donnee import connexion

# Jours de la semaine tels qu'ils sont stockés dans la table Programme.
JOURS_FR = [
    "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche",
]


class ErreurBaseDeDonnees(Exception):
    """Remontée au routeur pour produire une réponse 503 explicite."""


class TableManquante(ErreurBaseDeDonnees):
    """La table n'existe pas encore (migration non appliquée)."""


def _executer(sql, params=(), une_seule=False):
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                cursor.execute(sql, params)
                return cursor.fetchone() if une_seule else cursor.fetchall()
    except pymysql.err.ProgrammingError as e:
        # 1146 = table inconnue : la migration n'a pas encore été jouée.
        if e.args and e.args[0] == 1146:
            raise TableManquante(str(e)) from e
        raise ErreurBaseDeDonnees(str(e)) from e
    except pymysql.MySQLError as e:
        raise ErreurBaseDeDonnees(str(e)) from e


def _ecrire(sql, params=()):
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                cursor.execute(sql, params)
                lignes = cursor.rowcount
            conn.commit()
            return lignes
    except pymysql.err.ProgrammingError as e:
        if e.args and e.args[0] == 1146:
            raise TableManquante(str(e)) from e
        raise ErreurBaseDeDonnees(str(e)) from e
    except pymysql.MySQLError as e:
        raise ErreurBaseDeDonnees(str(e)) from e


def _texte(valeur):
    """Rend une valeur SQL sérialisable en JSON (time, timedelta, date…)."""
    if valeur is None:
        return None
    if isinstance(valeur, timedelta):
        total = int(valeur.total_seconds())
        signe = "-" if total < 0 else ""
        total = abs(total)
        return f"{signe}{total // 3600:02d}:{(total % 3600) // 60:02d}:{total % 60:02d}"
    if isinstance(valeur, (datetime, date)):
        return valeur.isoformat()
    return str(valeur)


# ---------------------------------------------------------------------------
# Authentification
# ---------------------------------------------------------------------------

def compte_utilisateur(identifiant):
    """Compte d'administration (table utilisateurs), mot de passe compris.

    La vérification du mot de passe est faite par l'appelant : ce module ne
    connaît pas la méthode de hachage.
    """
    ligne = _executer(
        """
        SELECT u.id, u.nom, u.mot_de_passe, r.nom AS role, u.IDSection
        FROM utilisateurs u
        JOIN roles r ON r.id = u.role_id
        WHERE u.nom = %s
        """,
        (identifiant,),
        une_seule=True,
    )
    if not ligne:
        return None
    return {
        "id": ligne[0],
        "nom": ligne[1],
        "mot_de_passe": ligne[2],
        "role": ligne[3],
        "section": ligne[4],
    }


def compte_employe(matricule):
    """Compte enseignant (table employe), identifié par son matricule."""
    ligne = _executer(
        """
        SELECT e.matricule, r.nom AS role, e.section
        FROM employe e
        JOIN roles r ON r.id = e.id_role
        WHERE e.matricule = %s
        """,
        (matricule,),
        une_seule=True,
    )
    if not ligne:
        return None
    return {"matricule": ligne[0], "role": ligne[1], "section": ligne[2]}


def enregistrer_mot_de_passe(id_utilisateur, empreinte):
    return _ecrire(
        "UPDATE utilisateurs SET mot_de_passe = %s WHERE id = %s",
        (empreinte, id_utilisateur),
    )


# ---------------------------------------------------------------------------
# Profil
# ---------------------------------------------------------------------------

def profil(matricule):
    ligne = _executer(
        """
        SELECT DISTINCT
            e.matricule, p.professeur_nom, e.telephone, e.email,
            e.adresse, e.poste, e.date_embauche, e.section
        FROM employe e
        LEFT JOIN Programme p ON p.professeur_code = e.matricule
        WHERE e.matricule = %s
        """,
        (matricule,),
        une_seule=True,
    )
    if not ligne:
        return None
    return {
        "matricule": ligne[0],
        "nom": ligne[1],
        "telephone": ligne[2],
        "email": ligne[3],
        "adresse": ligne[4],
        "poste": ligne[5],
        "date_embauche": _texte(ligne[6]),
        "section": ligne[7],
    }


def maj_profil(matricule, telephone, email, adresse):
    """Met à jour les seuls champs que l'enseignant peut modifier."""
    return _ecrire(
        """
        UPDATE employe
        SET telephone = %s, email = %s, adresse = %s
        WHERE matricule = %s
        """,
        (telephone, email, adresse, matricule),
    )


# ---------------------------------------------------------------------------
# Pointages
# ---------------------------------------------------------------------------

def _journees(matricule, debut=None, fin=None, limite=None):
    """Pointages regroupés par journée : première entrée, dernier départ."""
    conditions = ["p.IDEmploye = %s"]
    params = [matricule]
    if debut:
        conditions.append("DATE(p.date_pointage) >= %s")
        params.append(debut)
    if fin:
        conditions.append("DATE(p.date_pointage) <= %s")
        params.append(fin)

    sql = f"""
        SELECT
            DATE(p.date_pointage) AS jour,
            MIN(TIME(p.date_pointage)) AS entree,
            MAX(TIME(p.date_pointage)) AS sortie,
            COUNT(*) AS nb_passages,
            TIMEDIFF(MAX(p.date_pointage), MIN(p.date_pointage)) AS duree
        FROM pointages p
        WHERE {' AND '.join(conditions)}
        GROUP BY DATE(p.date_pointage)
        ORDER BY jour DESC
    """
    if limite:
        sql += " LIMIT %s"
        params.append(int(limite))

    lignes = _executer(sql, tuple(params))
    return [
        {
            "date": _texte(ligne[0]),
            "entree": _texte(ligne[1]),
            "sortie": _texte(ligne[2]) if ligne[3] > 1 else None,
            "nb_passages": ligne[3],
            "duree": _texte(ligne[4]) if ligne[3] > 1 else None,
            "complet": ligne[3] >= 2,
        }
        for ligne in lignes
    ]


def historique(matricule, debut=None, fin=None, limite=60):
    return _journees(matricule, debut, fin, limite)


def journee_courante(matricule):
    aujourdhui = date.today().isoformat()
    journees = _journees(matricule, aujourdhui, aujourdhui)
    if not journees:
        return {
            "date": aujourdhui,
            "entree": None,
            "sortie": None,
            "nb_passages": 0,
            "duree": None,
            "complet": False,
            "statut": "absent",
        }
    journee = journees[0]
    journee["statut"] = "termine" if journee["complet"] else "present"
    return journee


def statistiques(matricule, mois=None):
    """Statistiques personnelles sur un mois (par défaut le mois courant).

    `mois` attendu au format AAAA-MM.
    """
    reference = date.today()
    if mois:
        annee, numero = (int(part) for part in mois.split("-")[:2])
        reference = date(annee, numero, 1)

    debut = reference.replace(day=1)
    fin = (debut + timedelta(days=32)).replace(day=1) - timedelta(days=1)

    ligne = _executer(
        """
        SELECT
            COUNT(*) AS jours_pointes,
            SUM(CASE WHEN nb >= 2 THEN 1 ELSE 0 END) AS jours_complets,
            SUM(TIME_TO_SEC(TIMEDIFF(sortie, entree))) AS secondes
        FROM (
            SELECT
                DATE(date_pointage) AS jour,
                COUNT(*) AS nb,
                MIN(date_pointage) AS entree,
                MAX(date_pointage) AS sortie
            FROM pointages
            WHERE IDEmploye = %s
              AND DATE(date_pointage) BETWEEN %s AND %s
            GROUP BY DATE(date_pointage)
        ) AS journees
        """,
        (matricule, debut.isoformat(), fin.isoformat()),
        une_seule=True,
    )

    jours_pointes = (ligne[0] or 0) if ligne else 0
    jours_complets = int(ligne[1] or 0) if ligne else 0
    secondes = int(ligne[2] or 0) if ligne else 0

    return {
        "mois": debut.strftime("%Y-%m"),
        "jours_pointes": jours_pointes,
        "jours_complets": jours_complets,
        "jours_incomplets": max(jours_pointes - jours_complets, 0),
        "heures_cumulees": round(secondes / 3600, 2),
        "duree_moyenne_heures": round(secondes / 3600 / jours_complets, 2) if jours_complets else 0,
    }


# ---------------------------------------------------------------------------
# Emploi du temps
# ---------------------------------------------------------------------------

def programme(matricule):
    lignes = _executer(
        """
        SELECT professeur_code, professeur_nom, jour, Matiere,
               heure_arrivee, heure_depart, duree_cours, section, type_edt
        FROM Programme
        WHERE professeur_code = %s
        ORDER BY heure_arrivee
        """,
        (matricule,),
    )
    return [
        {
            "matricule": ligne[0],
            "enseignant": ligne[1],
            "jour": _texte(ligne[2]),
            "matiere": ligne[3],
            "heure_arrivee": _texte(ligne[4]),
            "heure_depart": _texte(ligne[5]),
            "duree": _texte(ligne[6]),
            "section": ligne[7],
            "type": ligne[8],
        }
        for ligne in lignes
    ]


def programme_du_jour(matricule, jour=None):
    libelle = (jour or JOURS_FR[date.today().weekday()]).strip().lower()
    return [c for c in programme(matricule) if (c["jour"] or "").strip().lower() == libelle]


def programme_semaine(matricule):
    """Emploi du temps regroupé par jour, dans l'ordre de la semaine."""
    semaine = {jour: [] for jour in JOURS_FR}
    for cours in programme(matricule):
        libelle = (cours["jour"] or "").strip().lower()
        for jour in JOURS_FR:
            if jour.lower() == libelle:
                semaine[jour].append(cours)
                break
    return semaine


# ---------------------------------------------------------------------------
# Notifications  (table créée par database/migrations/001_mobile.sql)
# ---------------------------------------------------------------------------

def notifications(matricule, limite=50):
    lignes = _executer(
        """
        SELECT id, titre, message, type, lue, date_creation
        FROM notifications_mobile
        WHERE matricule = %s
        ORDER BY date_creation DESC
        LIMIT %s
        """,
        (matricule, int(limite)),
    )
    return [
        {
            "id": ligne[0],
            "titre": ligne[1],
            "message": ligne[2],
            "type": ligne[3],
            "lue": bool(ligne[4]),
            "date": _texte(ligne[5]),
        }
        for ligne in lignes
    ]


def marquer_lue(matricule, id_notification):
    return _ecrire(
        "UPDATE notifications_mobile SET lue = 1 WHERE id = %s AND matricule = %s",
        (id_notification, matricule),
    )


def creer_notification(matricule, titre, message, type_notification="info"):
    """Utilisée par le serveur (ex. après un pointage) pour alimenter le mobile."""
    return _ecrire(
        """
        INSERT INTO notifications_mobile (matricule, titre, message, type)
        VALUES (%s, %s, %s, %s)
        """,
        (matricule, titre, message, type_notification),
    )


def enregistrer_appareil(matricule, jeton, plateforme):
    """Enregistre le jeton Firebase du téléphone pour les notifications push."""
    return _ecrire(
        """
        INSERT INTO appareils_mobiles (matricule, jeton_fcm, plateforme)
        VALUES (%s, %s, %s)
        ON DUPLICATE KEY UPDATE
            matricule = VALUES(matricule),
            plateforme = VALUES(plateforme),
            date_maj = CURRENT_TIMESTAMP
        """,
        (matricule, jeton, plateforme),
    )
