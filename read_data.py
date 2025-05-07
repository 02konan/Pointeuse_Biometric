import pymysql

def read_data_from_db():
    try:
        # Connexion à la base de données
        data_base = pymysql.connect(
            host="localhost",
            user="root",
            password="",
            database="ifsm_database"
        )

        with data_base.cursor() as cursor:
            # Requête POUR AFFICHER LE NOMBRE D'EMPLOYES
            sql1 = "SELECT COUNT(nom) FROM empreintes"
            sql2 = """SELECT COUNT(*) AS nb_personnes
                      FROM (
                      SELECT user_id
                      FROM empreintes_utilisees
                      WHERE DATE(heure_pointage) = CURRENT_DATE()
                      GROUP BY user_id
                      HAVING 
                      MIN(TIME(heure_pointage)) <= '08:00:00'
                      AND MAX(TIME(heure_pointage)) >= '16:00:00'
                      ) AS personnes_presentes;
                   """
            sql3 = "SELECT COUNT(*) FROM empreintes_utilisees WHERE DATE(heure_pointage) = CURRENT_DATE() AND TIME(heure_pointage) > '08:00:00';"
            sql4 ="""SELECT COUNT(*)
                     FROM empreintes_utilisees
                     JOIN empreintes ON empreintes.user_id = empreintes_utilisees.user_id
                     WHERE heure_pointage=CURRENT_DATE();"""
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
        data_base.close()
        

    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)
 

