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
            sql1 = "SELECT COUNT(Matricule) FROM empreintes"
            sql2 = """SELECT COUNT(*) AS nb_personnes
                      FROM (
                      SELECT IDEmploye
                      FROM pointages
                      WHERE DATE(date_pointage) = '2025-01-10'
                      GROUP BY IDEmploye
                      HAVING 
                      MIN(TIME(date_pointage)) <= '08:00:00'
                      AND MAX(TIME(date_pointage)) >= '16:00:00'
                      ) AS personnes_presentes;
                   """
            sql3 = "SELECT COUNT(*) FROM pointages WHERE DATE(date_pointage) = '2025-01-10' AND TIME(date_pointage) > '08:00:00';"
            sql4 ="""SELECT COUNT(*)
                     FROM pointages
                     JOIN empreintes ON empreintes.IDEmploye = pointages.IDEmploye
                     WHERE date_pointage='2025-01-10';"""
            sql5 ="""SELECT DISTINCT empreintes.Matricule,pointages.date_pointage
FROM pointages
JOIN empreintes on pointages.IDEmploye=empreintes.IDEmploye
WHERE DATE(date_pointage)='2025-01-10'
ORDER BY date_pointage DESC LIMIT 4;"""
            cursor.execute(sql1)
            total_eleves=cursor.fetchone()[0]

            cursor.execute(sql2)
            total_Presents=cursor.fetchone()[0]

            cursor.execute(sql3)
            total_retard=cursor.fetchone()[0]

            cursor.execute(sql4)
            total_absents=cursor.fetchone()[0]

            cursor.execute(sql5)
            activité_recentes=cursor.fetchall()

            return total_eleves,total_Presents,total_retard,activité_recentes,total_absents
        data_base.close

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
def vefification_utilisateur():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql="SELECT * FROM utilisateurs"
                cursor.execute(sql)
                result=cursor.fetchall()
                return result
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur generale:", e)