from base_donnee import connexion
import pymysql

def creat_data_employee(idEmploye,nom, prenom, telephone, address, email, poste, photo_path, date,section):
    try:
        with connexion() as conn:
            with conn.cursor() as curseur:
                sql = """
                INSERT INTO professeur (Matricule,Nom, Prenom, Telephone, Adresse, Email, Poste, image, Date_Embauche,section)
                VALUES (%s,%s, %s, %s, %s, %s, %s, %s, %s, %s)
                """
                # Exécution de la requête
                curseur.execute(sql, (idEmploye,nom, prenom, telephone, address, email, poste, photo_path, date,section))
                print(curseur.rowcount, "enregistrement(s) inséré(s) avec succès.")
            # Validation de la transaction
            conn.commit()
    except pymysql.MySQLError as e:
        print(f"Erreur MySQL : {e}")
def creat_data_pointeuse(pointeuseN, pointeuseM,pointeuseP,Adresseip,pointeusePort,pointeuseSerie,pointeuseType):
    try:
        with connexion() as conn:
            with conn.cursor() as curseur:
                sql = """
                INSERT INTO pointeuse (Nom, Model,Localisation,AdresseIP, Port,  Serie, Type)
                VALUES (%s, %s, %s, %s, %s, %s, %s)
                """
                curseur.execute(sql, (pointeuseN, pointeuseM,pointeuseP,Adresseip,pointeusePort,pointeuseSerie,pointeuseType))
                print(curseur.rowcount, "enregistrement(s) inséré(s) avec succès.")
            conn.commit()
    except pymysql.MySQLError as e:
        print(f"Erreur MySQL : {e}")
