from programme.base_donnee import connexion
import pymysql
def creat_rapports(fichier, utilisateur,id_utilisateur,type):
    try:
        with connexion() as conn:
            with conn.cursor() as curseur:
                sql = "INSERT INTO rapports (fichier, utilisateur,id_utilisateur,type) VALUES (%s, %s, %s, %s)"
                curseur.execute(sql, (fichier, utilisateur,id_utilisateur,type))
            conn.commit()
            return True
    except pymysql.MySQLError as e:
        print(f"Erreur MySQL : {e}")
        return False

def creat_data_employee(idEmploye, Nom, telephone, address, email, poste, photo_path, date, section,role_id):
    try:
        with connexion() as conn:
            with conn.cursor() as curseur:
                curseur.execute("SELECT COUNT(*) FROM employe WHERE Matricule = %s", (idEmploye,))
                existe = curseur.fetchone()[0]

                if existe:
                    sql = """
                    UPDATE employe
                    SET Nom=%s, Telephone=%s, Adresse=%s, Email=%s, Poste=%s, image=%s, Date_Embauche=%s, section=%s,id_role=%s
                    WHERE Matricule=%s
                    """
                    curseur.execute(sql,(Nom ,telephone, address, email, poste, photo_path, date, section,role_id, idEmploye))
                    print("Employé mis à jour avec succès.")
                else:
                    sql = """
                    INSERT INTO employe (Matricule, Nom, Telephone, Adresse, Email, Poste, image, Date_Embauche, section,id_role)
                    VALUES (%s, %s,%s, %s, %s, %s, %s, %s, %s,%s)
                    """
                    curseur.execute(sql, (idEmploye, Nom, telephone, address, email, poste, photo_path, date, section,role_id))
                    print("Nouvel employé inséré avec succès.")

            conn.commit()
    except pymysql.MySQLError as e:
        print(f"Erreur MySQL : {e}")
def creat_data_pointeuse(pointeuseN, pointeuseM, pointeuseP, Adresseip,pointeuseSerie, pointeuseType):
    try:
        with connexion() as conn:
            with conn.cursor() as curseur:
                sql = """
                INSERT INTO pointeuse (`NomPointeuse`, `Model`, `Emplacement`, `AdresseIP`,`Serie`, `Type`)
                VALUES (%s, %s, %s, %s, %s, %s)
                """
                curseur.execute(sql, (pointeuseN, pointeuseM, pointeuseP, Adresseip,pointeuseSerie, pointeuseType))
                print(curseur.rowcount, "enregistrement(s) inséré(s) avec succès.")
            conn.commit()
    except pymysql.MySQLError as e:
        print(f"Erreur MySQL : {e}")
def cret_User(Nom, Email, password, role, IDsection):
    try:
        with connexion() as conn:
            with conn.cursor() as curseur:
                curseur.execute("SELECT COUNT(*) FROM utilisateurs WHERE id = %s", (id,))
                existe = curseur.fetchone()[0]
                if existe:
                    sql = """
                    UPDATE utilisateurs
                    SET nom=%s, email=%s,role_id=%s, IDSection=%s
                    WHERE id=%s
                    """
                    curseur.execute(sql, (Nom, Email, password, role, IDsection, id))
                    print("Utilisateur mis à jour avec succès.")
                else:
                    sql = """
                    INSERT INTO `utilisateurs` (`nom`, `email`, `mot_de_passe`, `role_id`, `IDSection`)
                    VALUES (%s, %s, %s, %s, %s)
                    """
                    curseur.execute(sql, (Nom, Email, password, role, IDsection))
                    print("Nouvel utilisateur inséré avec succès.")
            conn.commit()
    except pymysql.MySQLError as e:
        print(f"Erreur MySQL : {e}")