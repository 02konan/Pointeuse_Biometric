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
                WHERE u.nom =%s
                """
                cursor.execute(sql, (utilisateur_name,))
                result = cursor.fetchall()
                return result
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)
def read_matricule():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = "SELECT DISTINCT(Matricule) FROM empreintes"
                cursor.execute(sql)
                result = cursor.fetchall()
                return [row[0] for row in result]
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)
def read_data_from_db(section_name):
    try:
        data_base = connexion()

        with data_base.cursor() as cursor:
            # DONNÉES GÉNÉRALES
            sql1 = """
SELECT COUNT(e.Matricule) AS total_employes
FROM empreintes e
JOIN pointeuse pt ON pt.idPointeuse = e.IDPointeuse
JOIN section s ON s.idPointeuse = pt.idPointeuse
WHERE s.IDSection =%s;
"""
            sql2 = """SELECT COUNT(*) AS nb_presences
FROM (
  SELECT p.IDEmploye
  FROM pointages p
  JOIN empreintes e ON e.IDEmploye = p.IDEmploye
  JOIN section s ON s.idPointeuse = p.idPointeuse
  WHERE DATE(p.date_pointage) = CURRENT_DATE()
    AND s.IDSection = %s
  GROUP BY p.IDEmploye, DATE(p.date_pointage), e.Matricule
  HAVING COUNT(*) >= 2
) AS liste_presences;
                   """
            
            sql3 = """SELECT COUNT(*) AS nb_retardataires
FROM (
  SELECT p.IDEmploye
  FROM pointages p
  JOIN section s ON s.idPointeuse = p.idPointeuse
  WHERE DATE(p.date_pointage) = CURRENT_DATE()
    AND s.IDSection =%s AND p.IDEmploye!=0
  GROUP BY p.IDEmploye
  HAVING MIN(TIME(p.date_pointage)) > '08:00:00'
) AS retardataires;

"""
            
            sql4 = """SELECT COUNT(*)
FROM empreintes
LEFT JOIN pointages ON pointages.IDEmploye = empreintes.IDEmploye
  AND DATE(pointages.date_pointage) = CURRENT_DATE()
LEFT JOIN pointeuse ON pointeuse.idPointeuse = empreintes.IDPointeuse
LEFT JOIN section ON section.idPointeuse = pointeuse.idPointeuse
WHERE section.IDSection =%s AND pointages.IDEmploye IS NULL;

"""
            
            sql5 = """SELECT DISTINCT e.Matricule, p.date_pointage
FROM pointages p
JOIN empreintes e ON p.IDEmploye = e.IDEmploye
JOIN pointeuse pt ON pt.idPointeuse = p.idPointeuse
JOIN section s ON s.idPointeuse = pt.idPointeuse
WHERE DATE(p.date_pointage) = CURRENT_DATE()
  AND s.IDSection =%s
ORDER BY p.date_pointage DESC
LIMIT 5;
"""

            # NOUVELLES REQUÊTES PAR MOIS (Février 2025)
            sql6_mois = """SELECT COUNT(DISTINCT sous.IDEmploye) AS employes_actifs_mois
FROM (
    SELECT p.IDEmploye
    FROM pointages p
    JOIN empreintes e ON e.IDEmploye = p.IDEmploye
    JOIN pointeuse pt ON pt.idPointeuse = p.idPointeuse
    JOIN section s ON s.idPointeuse = pt.idPointeuse
    WHERE YEAR(p.date_pointage) = YEAR(CURRENT_DATE())
      AND MONTH(p.date_pointage) = MONTH(CURRENT_DATE())
      AND s.IDSection =%s
    GROUP BY p.IDEmploye, DATE(p.date_pointage)
    HAVING COUNT(p.date_pointage) >= 2
) AS sous;"""
            
            sql7_mois = """SELECT COUNT(DISTINCT DATE(p.date_pointage)) AS jours_travailles
FROM pointages p
JOIN pointeuse pt ON pt.idPointeuse = p.idPointeuse
JOIN section s ON s.idPointeuse = pt.idPointeuse
WHERE YEAR(p.date_pointage) = YEAR(CURRENT_DATE())
  AND MONTH(p.date_pointage) = MONTH(CURRENT_DATE())
  AND s.IDSection =%s;
"""
            
            sql8_mois = """SELECT COUNT(DISTINCT p.IDEmploye) AS employes_retard_mois
FROM pointages p
JOIN pointeuse pt ON pt.idPointeuse = p.idPointeuse
JOIN section s ON s.idPointeuse = pt.idPointeuse
WHERE YEAR(p.date_pointage) = 2025
  AND MONTH(p.date_pointage) = MONTH(CURRENT_DATE())
  AND TIME(p.date_pointage) > '08:30:00'
  AND NOT EXISTS (
    SELECT 1
    FROM pointages p2
    WHERE p2.IDEmploye = p.IDEmploye
      AND DATE(p2.date_pointage) = DATE(p.date_pointage)
      AND TIME(p2.date_pointage) <= '08:30:00'
  )
  AND s.IDSection =%s;
"""
            
            sql9_mois = """SELECT AVG(presents_par_jour) AS moyenne_presence_mois
                          FROM (
                              SELECT DATE(date_pointage) as jour, COUNT(DISTINCT IDEmploye) as presents_par_jour
                              FROM pointages
                              WHERE YEAR(date_pointage) = 2025 AND MONTH(date_pointage) = 4
                              GROUP BY DATE(date_pointage)
                          ) AS stats_quotidiennes;"""
            
            sql10_mois = """SELECT COUNT(*) AS total_pointages_mois
                           FROM pointages
                           WHERE YEAR(date_pointage) = 2025 AND MONTH(date_pointage) = 4;"""
           
            # EXÉCUTION DES REQUÊTES QUOTIDIENNES
            cursor.execute(sql1, (section_name,))
            total_eleves = cursor.fetchone()[0]

            cursor.execute(sql2, (section_name,))
            total_Presents = cursor.fetchone()[0]

            cursor.execute(sql3, (section_name,))
            total_retard = cursor.fetchone()[0]

            cursor.execute(sql4, (section_name,))
            total_absents = cursor.fetchone()[0]

            cursor.execute(sql5, (section_name,))
            activité_recentes = cursor.fetchall()

            # EXÉCUTION DES REQUÊTES MENSUELLES
            cursor.execute(sql6_mois, (section_name,))
            employes_actifs_mois = cursor.fetchone()[0]

            cursor.execute(sql7_mois, (section_name,))
            jours_travailles_mois = cursor.fetchone()[0]

            cursor.execute(sql8_mois, (section_name,))
            employes_retard_mois = cursor.fetchone()[0]

            return (
                # Données quotidiennes
                total_eleves, total_Presents, total_retard, activité_recentes, total_absents,
                # Données mensuelles
                employes_actifs_mois, jours_travailles_mois, employes_retard_mois
            )
            
        data_base.close()

    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)
def read_data_employe():
 try:
    with connexion() as conn:
        with conn.cursor() as cursor:
            sql="SELECT * FROM employe"
            cursor.execute(sql)
            result=cursor.fetchall()
    return result         
 except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
 except Exception as e:
        print("Erreur générale :", e)       
def read_data_presence(section_name):
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql="""SELECT
  eu.IDEmploye,
  e.Matricule,
  DATE(eu.date_pointage) AS date_pointage,
  MIN(TIME(eu.date_pointage)) AS heure_arrivee,
  MAX(TIME(eu.date_pointage)) AS heure_depart,
  TIMEDIFF(MAX(eu.date_pointage), MIN(eu.date_pointage)) AS temps_presence
  FROM pointages eu
  JOIN empreintes e ON e.IDEmploye = eu.IDEmploye
  JOIN pointeuse pt ON pt.idPointeuse = eu.idPointeuse
  JOIN section s ON s.idPointeuse = pt.idPointeuse
  WHERE DATE(eu.date_pointage) IN (CURRENT_DATE(), DATE_SUB(CURRENT_DATE(), INTERVAL 1 DAY))
  AND s.IDSection = %s
  GROUP BY eu.IDEmploye, DATE(eu.date_pointage), e.Matricule
  HAVING COUNT(*) >= 2
  ORDER BY date_pointage DESC, temps_presence;
                      """
                cursor.execute(sql, (section_name,))
                result=cursor.fetchall()
                return result
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)
def read_data_pointeuse():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql="SELECT * FROM pointeuse"
                cursor.execute(sql)
                result=cursor.fetchall()
                return result
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)
def verification_utilisateur(username, password):
    
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = """
                SELECT utilisateurs.id as identifiants, utilisateurs.nom as nom_utilisateur, roles.nom as nom_roles, utilisateurs.mot_de_passe, roles.id as role_id, utilisateurs.IDSection as id_section
                FROM utilisateurs
                JOIN roles ON utilisateurs.role_id = roles.id
                WHERE utilisateurs.nom = %s AND utilisateurs.mot_de_passe = %s
                """
                cursor.execute(sql, (username, password))
                result = cursor.fetchone()
                if result:
                    # Retourne un dictionnaire avec les infos utiles
                    return {
                        'identifiants': result[0],
                        'nom_utilisateur': result[1],
                        'nom_roles': result[2],
                        'mot_de_passe': result[3],
                        'role_id': result[4],
                        'id_section': result[5]
                    }
                else:
                    return None
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
        return None
    except Exception as e:
        print("Erreur générale :", e)
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
                sql = "SELECT IDSection, NomSection FROM section"
                cursor.execute(sql)
                result = cursor.fetchall()
                return result
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)
def read_idrole():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = "SELECT id, nom FROM roles"
                cursor.execute(sql)
                result = cursor.fetchall()  # récupère toutes les lignes
                return result  # renvoie une liste de tuples (id, nom)
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
        return []
    except Exception as e:
        print("Erreur générale :", e)
        return []
def read_utilisateur():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql="""SELECT utilisateurs.id,utilisateurs.nom, email,mot_de_passe,roles.nom as roles ,Nomsection
                 FROM utilisateurs
                 join section on section.IDSection=utilisateurs.IDSection
                 join roles on roles.id=utilisateurs.role_id"""
                cursor.execute(sql)
                result=cursor.fetchall()
                return result
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)
def generer_presence(date_debut, date_fin,section_name):
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = """
                SELECT 
  eu.IDEmploye,
  e.Matricule,
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
GROUP BY eu.IDEmploye, DATE(eu.date_pointage), e.Matricule
HAVING COUNT(*) >= 2
ORDER BY date_pointage;

                """
                cursor.execute(sql, (date_debut, date_fin,section_name))
                result = cursor.fetchall()
                return result
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)
def generer_retard(date_debut_retard, date_fin_retard,section_name):
   try:
    with connexion() as conn:
        with conn.cursor() as cursor:
           
            sql = """
             SELECT 
  eu.IDEmploye,
  e.Matricule,
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
GROUP BY eu.IDEmploye, DATE(eu.date_pointage), e.Matricule
HAVING 
  COUNT(*) >= 2
  AND MIN(TIME(eu.date_pointage)) BETWEEN '08:00:00' AND '18:30:00'
ORDER BY date_pointage;

             """
            cursor.execute(sql,(date_debut_retard,date_fin_retard,section_name))
            result = cursor.fetchall()
            return result
   except pymysql.MySQLError as e:
    print("Erreur MySQL :", e)
   except Exception as e:
    print("Erreur générale :", e)
def generer_absence(date_debut_absence, date_fin_absence,section_name):
    try:
     with connexion() as conn:
        with conn.cursor() as cursor:
            sql = """
            SELECT 
  eu.IDEmploye,
  e.Matricule,
  DATE(eu.date_pointage) AS date_pointage,
  MIN(TIME(eu.date_pointage)) AS heure_arrivee,
  MAX(TIME(eu.date_pointage)) AS heure_depart,
  TIMEDIFF(MAX(eu.date_pointage), MIN(eu.date_pointage)) AS temps_presence
FROM pointages eu
JOIN empreintes e ON e.IDEmploye = eu.IDEmploye
JOIN pointeuse p ON p.idPointeuse = e.IDPointeuse
JOIN section s ON s.idPointeuse = p.idPointeuse
WHERE DATE(eu.date_pointage) BETWEEN %s AND %s
  AND s.IDSection = %s
  AND eu.IDEmploye != 0
GROUP BY eu.IDEmploye, DATE(eu.date_pointage), e.Matricule
HAVING COUNT(*) < 2
ORDER BY date_pointage;

            """
            cursor.execute(sql,(date_debut_absence,date_fin_absence,section_name))
            result = cursor.fetchall()
            return result
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
              eu.IDEmploye,
              e.Matricule,
              DATE(eu.date_pointage) AS Date_pointage,
              MIN(TIME(eu.date_pointage)) AS date_arrivee,
              MAX(TIME(eu.date_pointage)) AS date_depart,
              TIMEDIFF(MAX(eu.date_pointage), MIN(eu.date_pointage)) AS temps_presence
            FROM pointages eu
            JOIN empreintes e ON e.IDEmploye = eu.IDEmploye
            WHERE e.Matricule = %s
            GROUP BY eu.IDEmploye, DATE(eu.date_pointage), e.Matricule
            HAVING COUNT(DISTINCT eu.date_pointage) >= 2
            ORDER BY Date_pointage;
            """
            cursor.execute(sql, (Matricule))
            result = cursor.fetchall()
            return result 
  except pymysql.MySQLError as e:
     print("Erreur MySQL :", e)
  except Exception as e:
     print("Erreur générale :", e)