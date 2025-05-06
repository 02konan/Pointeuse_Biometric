import pymysql

def creat_data_employee(nom, prenom, telephone, email):
    try:
        conn = pymysql.connect(
            host="localhost",
            user="root",
            password="",
            database="ifsm_database"
        )
        curseur = conn.cursor()
        sql = "INSERT INTO professeur (Nom, Prenom, Telephone, Email) VALUES (%s, %s, %s, %s)"
        curseur.execute(sql, (nom, prenom, telephone, email))
        conn.commit()
    except pymysql.MySQLError as e:
        print(f"Erreur MySQL : {e}")
    finally:
        conn.close()