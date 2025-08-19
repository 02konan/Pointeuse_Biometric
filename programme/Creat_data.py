from programme.base_donnee import connexion
import pymysql
def creat_rapports(fichier, utilisateur,id_utilisateur):
    try:
        with connexion() as conn:
            with conn.cursor() as curseur:
                sql = "INSERT INTO rapports (fichier, utilisateur,id_utilisateur) VALUES (%s, %s, %s)"
                print(f"Tentative insertion: {fichier}, {utilisateur}")
                curseur.execute(sql, (fichier, utilisateur,id_utilisateur))
            conn.commit()
            print("Insertion réussie")
            return True
    except pymysql.MySQLError as e:
        print(f"Erreur MySQL : {e}")
        return False

def creat_data_employee(idEmploye, nom, prenom, telephone, address, email, poste, photo_path, date, section):
    try:
        with connexion() as conn:
            with conn.cursor() as curseur:
                curseur.execute("SELECT COUNT(*) FROM Employe WHERE Matricule = %s", (idEmploye,))
                existe = curseur.fetchone()[0]

                if existe:
                    sql = """
                    UPDATE Employe
                    SET Nom=%s, Prenom=%s, Telephone=%s, Adresse=%s, Email=%s, Poste=%s, image=%s, Date_Embauche=%s, section=%s
                    WHERE Matricule=%s
                    """
                    curseur.execute(sql, (nom, prenom, telephone, address, email, poste, photo_path, date, section, idEmploye))
                    print("Employé mis à jour avec succès.")
                else:
                    sql = """
                    INSERT INTO Employe (Matricule, Nom, Prenom, Telephone, Adresse, Email, Poste, image, Date_Embauche, section)
                    VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
                    """
                    curseur.execute(sql, (idEmploye, nom, prenom, telephone, address, email, poste, photo_path, date, section))
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