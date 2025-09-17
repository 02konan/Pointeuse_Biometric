import pymysql
from programme.base_donnee import connexion


def read_raports(utilisateur_name):
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = """
                SELECT r.fichier, u.nom
                FROM rapports r
                JOIN utilisateurs u ON r.id_utilisateur = u.id
                WHERE u.nom = %s
                """
                cursor.execute(sql, (utilisateur_name,))
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

def read_matricule():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = "SELECT DISTINCT(Nom) FROM empreintes"
                cursor.execute(sql)
                result = cursor.fetchall()
                return [row[0] for row in result]
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

def read_data_from_db(section_name):
    data_base = connexion()
    try:
        with data_base.cursor() as cursor:
            sql1 = """
SELECT COUNT(e.Nom) AS total_employes
FROM empreintes e
JOIN pointeuse pt ON pt.idPointeuse = e.IDPointeuse
JOIN section s ON s.idPointeuse = pt.idPointeuse
WHERE s.IDSection = %s;
"""
            sql2 = """SELECT COUNT(*) AS nb_presences
FROM (
    SELECT p.IDEmploye
    FROM pointages p
    JOIN empreintes e ON e.IDEmploye = p.IDEmploye   
    JOIN pointeuse pt ON pt.idPointeuse = p.idPointeuse
    JOIN section s ON s.idPointeuse = pt.idPointeuse
    WHERE DATE(p.date_pointage) = CURRENT_DATE()
    AND s.IDSection =%s 
    GROUP BY p.IDEmploye
    HAVING COUNT(p.IDEmploye) >= 2
) AS liste_presences;
"""
            sql3 = """SELECT COUNT(*) AS nb_retardataires
FROM (
    SELECT p.IDEmploye
    FROM pointages p
    JOIN section s ON s.idPointeuse = p.idPointeuse
    WHERE DATE(p.date_pointage) = CURRENT_DATE()
      AND s.IDSection =%s
      AND p.IDEmploye IS NOT NULL
    GROUP BY p.IDEmploye
    HAVING MIN(TIME(p.date_pointage)) > '08:00:00'
) AS retardataires;

"""
            sql4 = """SELECT COUNT(*) AS nb_absents
FROM empreintes e
LEFT JOIN pointages p 
       ON p.IDEmploye = e.IDEmploye 
      AND DATE(p.date_pointage) = CURRENT_DATE()
LEFT JOIN pointeuse pt 
       ON pt.idPointeuse = e.IDPointeuse
LEFT JOIN section s 
       ON s.idPointeuse = pt.idPointeuse
WHERE s.IDSection = %s
  AND p.IDEmploye IS NULL;
;
"""
            sql5 = """SELECT DISTINCT e.Nom, p.date_pointage,p.Status
FROM pointages p
JOIN empreintes e ON p.IDEmploye = e.IDEmploye
JOIN pointeuse pt ON pt.idPointeuse = p.idPointeuse
JOIN section s ON s.idPointeuse = pt.idPointeuse
WHERE DATE(p.date_pointage) = CURRENT_DATE()
    AND s.IDSection =%s
ORDER BY p.date_pointage DESC
LIMIT 10;
"""
            # Actifs du mois
            sql6_mois = """
SELECT COUNT(DISTINCT sous.IDEmploye) AS employes_actifs_mois
FROM (
        SELECT p.IDEmploye
        FROM pointages p
        JOIN empreintes e ON e.IDEmploye = p.IDEmploye
        JOIN pointeuse pt ON pt.idPointeuse = p.idPointeuse
        JOIN section s ON s.idPointeuse = pt.idPointeuse
        WHERE YEAR(p.date_pointage) = YEAR(CURRENT_DATE())
            AND MONTH(p.date_pointage) = MONTH(CURRENT_DATE())
            AND s.IDSection = %s
        GROUP BY p.IDEmploye, DATE(p.date_pointage)
        HAVING COUNT(p.date_pointage) >= 2
) AS sous;
"""
            # Jours travaillés dans le mois
            sql7_mois = """
SELECT COUNT(DISTINCT DATE(p.date_pointage)) AS jours_travailles
FROM pointages p
JOIN pointeuse pt ON pt.idPointeuse = p.idPointeuse
JOIN section s ON s.idPointeuse = pt.idPointeuse
WHERE YEAR(p.date_pointage) = YEAR(CURRENT_DATE())
    AND MONTH(p.date_pointage) = MONTH(CURRENT_DATE())
    AND s.IDSection = %s;
"""
            # Retards du mois
            sql8_mois = """
SELECT COUNT(DISTINCT p.IDEmploye) AS employes_retard_mois
FROM pointages p
JOIN pointeuse pt ON pt.idPointeuse = p.idPointeuse
JOIN section s ON s.idPointeuse = pt.idPointeuse
WHERE YEAR(p.date_pointage) = YEAR(CURRENT_DATE())
    AND MONTH(p.date_pointage) = MONTH(CURRENT_DATE())
    AND TIME(p.date_pointage) > '08:30:00'
    AND NOT EXISTS (
        SELECT 1
        FROM pointages p2
        WHERE p2.IDEmploye = p.IDEmploye
            AND DATE(p2.date_pointage) = DATE(p.date_pointage)
            AND TIME(p2.date_pointage) <= '08:30:00'
    )
    AND s.IDSection = %s;
"""
            # --- Exécution ---
            cursor.execute(sql1, (section_name,))
            total_eleves = cursor.fetchone()[0]

            cursor.execute(sql2, (section_name,))
            total_Presents = cursor.fetchone()[0]

            cursor.execute(sql3, (section_name,))
            total_retard = cursor.fetchone()[0]

            cursor.execute(sql4, (section_name,))
            total_absents = cursor.fetchone()[0]

            cursor.execute(sql5, (section_name,))
            activite_recentes = cursor.fetchall()

            cursor.execute(sql6_mois, (section_name,))
            employes_actifs_mois = cursor.fetchone()[0]

            cursor.execute(sql7_mois, (section_name,))
            jours_travailles_mois = cursor.fetchone()[0]

            cursor.execute(sql8_mois, (section_name,))
            employes_retard_mois = cursor.fetchone()[0]

            return (
                    total_eleves, total_Presents, total_retard, activite_recentes, total_absents,
                    employes_actifs_mois, jours_travailles_mois, employes_retard_mois
            )
    except Exception as e:
        print("Erreur lors de la lecture des données du tableau de bord :", e)
        return None

def read_data_employe():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = "SELECT * FROM employe"
                cursor.execute(sql)
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

def read_data_presence(section_name):
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql="""SELECT
    pr.professeur_code,
    pr.professeur_nom,
    arrivee,
    depart,
    p.jour_pointage,
    DATE(p.date_pointage) AS date_pointage,
    Duree_initial,
    pp.Status,
    pr.heure_arrivee,
    TIMEDIFF(depart,arrivee) AS temps_presence
FROM pointage_programe pp
JOIN Programme pr ON pr.IDProgramme = pp.IDProgramme
JOIN pointages p ON pp.IDPointage = p.id
JOIN pointeuse pt ON pt.idPointeuse = p.idPointeuse
JOIN section s ON s.idPointeuse = pt.idPointeuse
WHERE DATE(p.date_pointage)=CURDATE()
  AND s.IDSection = %s
GROUP BY pr.professeur_code, pr.professeur_nom, DATE(p.date_pointage)
ORDER BY p.date_pointage DESC, temps_presence;
                      """
                cursor.execute(sql, (section_name,))
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

def read_data_pointeuse():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                cursor.execute("SELECT * FROM pointeuse")
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

def verification_utilisateur(username, password):
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = """
                SELECT utilisateurs.id as identifiants, utilisateurs.nom as nom_utilisateur, 
                       roles.nom as nom_roles, utilisateurs.mot_de_passe, 
                       roles.id as role_id, utilisateurs.IDSection as id_section
                FROM utilisateurs
                JOIN roles ON utilisateurs.role_id = roles.id
                WHERE utilisateurs.nom = %s AND utilisateurs.mot_de_passe = %s
                """
                cursor.execute(sql, (username, password))
                result = cursor.fetchone()
                if result:
                    return {
                        'identifiants': result[0],
                        'nom_utilisateur': result[1],
                        'nom_roles': result[2],
                        'mot_de_passe': result[3],
                        'role_id': result[4],
                        'id_section': result[5]
                    }
                return None
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
        return None
    except Exception as e:
        print("Erreur générale :", e)
        return None

def read_idsection():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                cursor.execute("SELECT IDSection, NomSection FROM section")
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

def read_idrole():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                cursor.execute("SELECT id, nom FROM roles")
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

def read_utilisateur():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = """
                SELECT utilisateurs.id, utilisateurs.nom, email, mot_de_passe, 
                       roles.nom as roles, Nomsection
                FROM utilisateurs
                JOIN section ON section.IDSection = utilisateurs.IDSection
                JOIN roles ON roles.id = utilisateurs.role_id
                """
                cursor.execute(sql)
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

def generer_presence(date_debut, date_fin, section_name):
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = """
                SELECT
                  e.Nom,
                  DATE(eu.date_pointage) AS date_pointage,
                  MIN(TIME(eu.date_pointage)) AS heure_arrivee,
                  MAX(TIME(eu.date_pointage)) AS heure_depart,
                  TIMEDIFF(MAX(eu.date_pointage), MIN(eu.date_pointage)) AS temps_presence
                FROM pointages eu
                JOIN empreintes e ON e.IDEmploye = eu.IDEmploye
                JOIN pointeuse pt ON pt.idPointeuse = eu.idPointeuse
                JOIN section s ON s.idPointeuse = pt.idPointeuse
                WHERE DATE(eu.date_pointage) BETWEEN %s AND %s
                  AND s.IDSection = %s
                GROUP BY eu.IDEmploye, DATE(eu.date_pointage), e.Nom
                HAVING COUNT(*) >= 2
                ORDER BY date_pointage;
                """
                cursor.execute(sql, (date_debut, date_fin, section_name))
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

def generer_retard(date_debut_retard, date_fin_retard, section_name):
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = """
                SELECT
                  e.Nom,
                  DATE(eu.date_pointage) AS date_pointage,
                  MIN(TIME(eu.date_pointage)) AS heure_arrivee,
                  MAX(TIME(eu.date_pointage)) AS heure_depart,
                  TIMEDIFF(MAX(eu.date_pointage), MIN(eu.date_pointage)) AS temps_presence
                FROM pointages eu
                JOIN empreintes e ON e.IDEmploye = eu.IDEmploye
                JOIN pointeuse pt ON e.IDPointeuse = pt.idPointeuse
                JOIN section s ON pt.idPointeuse = s.idPointeuse
                WHERE DATE(eu.date_pointage) BETWEEN %s AND %s
                  AND s.IDSection = %s
                GROUP BY eu.IDEmploye, DATE(eu.date_pointage), e.Nom
                HAVING COUNT(*) >= 2
                   AND MIN(TIME(eu.date_pointage)) > '08:00:00'
                ORDER BY date_pointage;
                """
                cursor.execute(sql, (date_debut_retard, date_fin_retard, section_name))
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

def generer_absence(date_debut_absence, date_fin_absence, section_name):
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = """
                SELECT
                  e.Nom,
                  DATE(eu.date_pointage) AS date_pointage,
                  MIN(TIME(eu.date_pointage)) AS heure_arrivee,
                  MAX(TIME(eu.date_pointage)) AS heure_depart,
                  TIMEDIFF(MAX(eu.date_pointage), MIN(eu.date_pointage)) AS temps_presence
                FROM pointages eu
                JOIN empreintes e ON e.IDEmploye = eu.IDEmploye
                JOIN pointeuse pt ON pt.idPointeuse = e.IDPointeuse
                JOIN section s ON s.idPointeuse = pt.idPointeuse
                WHERE DATE(eu.date_pointage) BETWEEN %s AND %s
                  AND s.IDSection = %s
                  AND eu.IDEmploye != 0
                GROUP BY eu.IDEmploye, DATE(eu.date_pointage), e.Nom
                HAVING COUNT(*) < 2
                ORDER BY date_pointage;
                """
                cursor.execute(sql, (date_debut_absence, date_fin_absence, section_name))
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

def generer_unique_presence(Matricule):
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = """
                SELECT
                  e.Nom,
                  DATE(eu.date_pointage) AS Date_pointage,
                  MIN(TIME(eu.date_pointage)) AS date_arrivee,
                  MAX(TIME(eu.date_pointage)) AS date_depart,
                  TIMEDIFF(MAX(eu.date_pointage), MIN(eu.date_pointage)) AS temps_presence
                FROM pointages eu
                JOIN empreintes e ON e.IDEmploye = eu.IDEmploye
                WHERE e.Matricule = %s
                GROUP BY eu.IDEmploye, DATE(eu.date_pointage), e.Matricule
                HAVING COUNT(*) >= 2
                ORDER BY Date_pointage;
                """
                cursor.execute(sql, (Matricule,))
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

def pointage_invalid(section_name):
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = """SELECT 
    p.professeur_code,
    p.professeur_nom,
    DATE(ptg.date_pointage),
    po.idPointeuse,
    TIME(ptg.date_pointage) AS heure_pointage,
    p.heure_arrivee,
    p.heure_depart,
    p.duree_cours,
    ptg.jour_pointage
FROM Programme p
INNER JOIN pointages ptg ON p.professeur_code = ptg.IDEmploye
    AND DATE(ptg.date_pointage) =CURDATE() AND ptg.jour_pointage=p.jour
INNER JOIN pointeuse po ON po.idPointeuse = ptg.idPointeuse
INNER JOIN section s ON s.idPointeuse = po.idPointeuse
WHERE s.IDSection =%s
GROUP BY 
    p.professeur_code,
    p.professeur_nom
    HAVING COUNT(DISTINCT ptg.id)=1
    ORDER BY p.professeur_code
                """
                cursor.execute(sql, (section_name,))
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
        return []

def pointeuse(section_name):
    try:
        with connexion() as conn:
            sql="""SELECT idPointeuse FROM pointeuse WHERE idPointeuse
                IN (SELECT idPointeuse FROM section WHERE IDSection = %s)"""
            with conn.cursor() as cursor:
                cursor.execute(sql,(section_name,))
                return cursor.fetchone()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
        return None