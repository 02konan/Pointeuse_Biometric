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
            sql2 = "SELECT COUNT(*) FROM `empreintes_utilisees` WHERE time(heure_pointage) < '07:00:00'"
            sql3 = "SELECT COUNT(*) FROM `empreintes_utilisees` WHERE time(heure_pointage) > '08:00:00'"
            sql4 =""
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
            cursor.execute(sql5)
            activité_recentes=cursor.fetchall()

            return total_eleves,total_Presents,total_retard,activité_recentes
        data_base.close()

    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)
# print(read_data_from_db)

