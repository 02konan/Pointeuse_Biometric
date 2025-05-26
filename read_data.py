import pymysql
from base_donnee import connexion

def read_matricule():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = "SELECT DISTINCT(nom) FROM empreintes"
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
            sql1 = "SELECT COUNT(nom) FROM empreintes"
            sql2 = """SELECT COUNT(*) AS nb_personnes
                      FROM (
                      SELECT user_id
                      FROM empreintes_utilisees
                      WHERE DATE(heure_pointage) = '4-05-2025'
                      GROUP BY user_id
                      HAVING 
                      MIN(TIME(heure_pointage)) <= '08:00:00'
                      AND MAX(TIME(heure_pointage)) >= '16:00:00'
                      ) AS personnes_presentes;
                   """
            sql3 = "SELECT COUNT(*) FROM empreintes_utilisees WHERE DATE(heure_pointage) = '4-05-2025' AND TIME(heure_pointage) > '08:00:00';"
            sql4 ="""SELECT COUNT(*)
                     FROM empreintes_utilisees
                     JOIN empreintes ON empreintes.user_id = empreintes_utilisees.user_id
                     WHERE heure_pointage='4-05-2025';"""
            sql5 = "SELECT DISTINCT empreintes.nom,heure_pointage " \
            "FROM empreintes_utilisees " \
            "JOIN empreintes on empreintes_utilisees.user_id=empreintes.user_id" \
            " ORDER BY heure_pointage DESC LIMIT 4;"
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
            sql="SELECT * FROM `professeur`"
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
  eu.user_id,
  e.nom,
  DATE(eu.heure_pointage) AS Date_pointage,
  MIN(TIME(eu.heure_pointage)) AS heure_arrivee,
  MAX(TIME(eu.heure_pointage)) AS heure_depart,
  TIMEDIFF(MAX(eu.heure_pointage), MIN(eu.heure_pointage)) AS temps_presence
FROM empreintes_utilisees eu
JOIN empreintes e ON e.user_id = eu.user_id
GROUP BY eu.user_id, DATE(eu.heure_pointage), e.nom
HAVING COUNT(DISTINCT eu.heure_pointage) >= 2
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
                sql="SELECT * FROM utilisateur"
                cursor.execute(sql)
                result=cursor.fetchall()
                return result
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur generale:", e)