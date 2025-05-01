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
            # Requête SQL CORRECTE avec WHERE
            sql = "SELECT * FROM empreintes WHERE user_id = %s"
            user_id = 164
            cursor.execute(sql, (user_id,))
            return cursor.fetchall()
        data_base.close()

    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

