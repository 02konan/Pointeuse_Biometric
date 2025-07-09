import pymysql
from base_donnee import connexion
def read_data_employe_matricule():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = """SELECT 
  pointages.date_pointage,
  employe.nom,
  employe.prenom,
  employe.matricule
FROM pointages
JOIN empreintes ON pointages.IDEmploye = empreintes.IDEmploye
JOIN employe ON empreintes.matricule = employe.matricule
WHERE employe.matricule = '18589259T';"""
                cursor.execute(sql)
                result = cursor.fetchall()
                return result
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)
def utilisateur():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = "SELECT matricule,nom FROM employe"
                cursor.execute(sql)
                result = cursor.fetchall()
                return result
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)