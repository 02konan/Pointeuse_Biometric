from base_donnee import connexion
import pymysql

def creat_data_employee(nom, prenom, telephone, address, email, poste, photo_path, date):
    try:
        with connexion() as conn:
            with conn.cursor() as curseur:
                sql = """
                INSERT INTO professeur (Nom, Prenom, Telephone, Adresse, Email, Poste, image, Date_Embauche)
                VALUES (%s, %s, %s, %s, %s, %s, %s, %s)
                """
                # Exécution de la requête
                curseur.execute(sql, (nom, prenom, telephone, address, email, poste, photo_path, date))
                print(curseur.rowcount, "enregistrement(s) inséré(s) avec succès.")
            # Validation de la transaction
            conn.commit()
    except pymysql.MySQLError as e:
        print(f"Erreur MySQL : {e}")
# def creat_data_empreinte(user_id, empreinte_path):