import pymysql
from base_donnee import connexion
def verification_prg(id_prof,nom_prof):
    
    try:
        db = connexion()
        cursor = db.cursor()

        cursor.execute("SELECT * FROM programme WHERE nom = %s", (id_prof,nom_prof))
        result = cursor.fetchone()

        cursor.close()
        db.close()

        return result[0] > 0

    except pymysql.MySQLError as e:
        print(f"Erreur lors de la vérification du programme: {e}")
        return False