import pymysql
from base_donnee import connexion
  
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
def read_data_from_db():
    try:
        data_base = connexion()

        with data_base.cursor() as cursor:
            # DONNÉES GÉNÉRALES
            sql1 = "SELECT COUNT(Matricule) FROM empreintes"
            
            # DONNÉES PAR JOUR (2025-02-10)
            sql2 = """SELECT COUNT(DISTINCT IDEmploye) AS nb_personnes
                      FROM (
                      SELECT IDEmploye
                      FROM pointages
                      WHERE DATE(date_pointage) = '2025-04-30'
                      GROUP BY IDEmploye
                      HAVING 
                      MIN(TIME(date_pointage)) <= '08:00:00'
                      AND MAX(TIME(date_pointage)) >= '16:30:00'
                      ) AS personnes_presentes;
                   """
            
            sql3 = "SELECT COUNT(DISTINCT IDEmploye) FROM pointages WHERE DATE(date_pointage) = '2025-04-30' AND TIME(date_pointage) > '08:00:00';"
            
            sql4 = """SELECT COUNT(*)
                     FROM empreintes e
                     LEFT JOIN pointages p ON e.IDEmploye = p.IDEmploye 
                         AND DATE(p.date_pointage) = '2025-04-30'
                     WHERE p.IDEmploye IS NULL;"""
            
            sql5 = """SELECT DISTINCT empreintes.Matricule,pointages.date_pointage
                     FROM pointages
                     JOIN empreintes on pointages.IDEmploye=empreintes.IDEmploye
                     WHERE DATE(date_pointage)='2025-04-30'
                     ORDER BY date_pointage DESC LIMIT 5;"""

            # NOUVELLES REQUÊTES PAR MOIS (Février 2025)
            sql6_mois = """SELECT COUNT(DISTINCT p.IDEmploye) AS employes_actifs_mois
                          FROM pointages p
                          WHERE YEAR(date_pointage) = 2025 AND MONTH(date_pointage) = 4;"""
            
            sql7_mois = """SELECT COUNT(DISTINCT DATE(date_pointage)) AS jours_travailles
                          FROM pointages
                          WHERE YEAR(date_pointage) = 2025 AND MONTH(date_pointage) = 4;"""
            
            sql8_mois = """SELECT COUNT(DISTINCT p.IDEmploye) AS employes_retard_mois
                          FROM pointages p
                          WHERE YEAR(date_pointage) = 2025 AND MONTH(date_pointage) = 4
                          AND TIME(date_pointage) > '08:30:00'
                          AND p.IDEmploye IN (
                              SELECT DISTINCT IDEmploye 
                              FROM pointages 
                              WHERE DATE(date_pointage) = DATE(p.date_pointage)
                              AND TIME(date_pointage) > '08:30:00'
                              AND NOT EXISTS (
                                  SELECT 1 FROM pointages p2 
                                  WHERE p2.IDEmploye = IDEmploye
                                  AND DATE(p2.date_pointage) = DATE(p.date_pointage)
                                  AND TIME(p2.date_pointage) <= '08:30:00'
                              )
                          );"""
            
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
            cursor.execute(sql1)
            total_eleves = cursor.fetchone()[0]

            cursor.execute(sql2)
            total_Presents = cursor.fetchone()[0]

            cursor.execute(sql3)
            total_retard = cursor.fetchone()[0]

            cursor.execute(sql4)
            total_absents = cursor.fetchone()[0]

            cursor.execute(sql5)
            activité_recentes = cursor.fetchall()

            # EXÉCUTION DES REQUÊTES MENSUELLES
            cursor.execute(sql6_mois)
            employes_actifs_mois = cursor.fetchone()[0]

            cursor.execute(sql7_mois)
            jours_travailles_mois = cursor.fetchone()[0]

            cursor.execute(sql8_mois)
            employes_retard_mois = cursor.fetchone()[0]

            cursor.execute(sql9_mois)
            moyenne_presence_mois = cursor.fetchone()[0]

            cursor.execute(sql10_mois)
            total_pointages_mois = cursor.fetchone()[0]

            return (
                # Données quotidiennes
                total_eleves, total_Presents, total_retard, activité_recentes, total_absents,
                # Données mensuelles
                employes_actifs_mois, jours_travailles_mois, employes_retard_mois, 
                moyenne_presence_mois, total_pointages_mois
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
            sql="SELECT * FROM `Employe`"
            cursor.execute(sql)
            result=cursor.fetchall()
    return result         
 except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
 except Exception as e:
        print("Erreur générale :", e)       
def read_data_presence():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql="""SELECT 
  eu.IDEmploye,
  e.Matricule,
  DATE(eu.date_pointage) AS Date_pointage,
  MIN(TIME(eu.date_pointage)) AS date_arrivee,
  MAX(TIME(eu.date_pointage)) AS date_depart,
  TIMEDIFF(MAX(eu.date_pointage), MIN(eu.date_pointage)) AS temps_presence
FROM pointages eu
JOIN empreintes e ON e.IDEmploye = eu.IDEmploye
WHERE DATE(eu.date_pointage)='2025-04-30'
GROUP BY eu.IDEmploye, DATE(eu.date_pointage), e.Matricule
HAVING COUNT(DISTINCT eu.date_pointage) >= 2
ORDER BY temps_presence;
                      """
                cursor.execute(sql)
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
                sql = "SELECT nom FROM utilisateurs WHERE nom = %s AND mot_de_passe = %s"
                cursor.execute(sql, (username, password))
                result = cursor.fetchone()
                return result is not None
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
        return False
    except Exception as e:
        print("Erreur générale :", e)
        return False

def generer_presence(date_debut, date_fin):
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
WHERE DATE(eu.date_pointage) BETWEEN %s AND %s
GROUP BY eu.IDEmploye, DATE(eu.date_pointage), e.Matricule
HAVING COUNT(DISTINCT eu.date_pointage) >= 2
ORDER BY Date_pointage;
                """
                cursor.execute(sql, (date_debut, date_fin))
                result = cursor.fetchall()
                return result
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)
def generer_retard(date_debut_retard, date_fin_retard):
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
             WHERE DATE(eu.date_pointage) BETWEEN %s AND %s
             GROUP BY eu.IDEmploye, DATE(eu.date_pointage), e.Matricule
              HAVING COUNT(DISTINCT eu.date_pointage) >= 2
             AND MIN(TIME(eu.date_pointage)) BETWEEN '08:00:00' AND '18:30:00'
             ORDER BY Date_pointage;
             """
            cursor.execute(sql,(date_debut_retard,date_fin_retard))
            result = cursor.fetchall()
            return result
   except pymysql.MySQLError as e:
    print("Erreur MySQL :", e)
   except Exception as e:
    print("Erreur générale :", e)
def generer_absence(date_debut_absence, date_fin_absence):
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
            WHERE DATE(eu.date_pointage) BETWEEN %s AND %s
            GROUP BY eu.IDEmploye, DATE(eu.date_pointage), e.Matricule
            HAVING COUNT(DISTINCT eu.date_pointage)< 2
            ORDER BY Date_pointage;
            """
            cursor.execute(sql,(date_debut_absence,date_fin_absence))
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