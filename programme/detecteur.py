import os
import pymysql
from zk import ZK, const
import time
from programme.base_donnee import connexion
def is_pingable(ip):
    response = os.system(f"ping -n 1 -w 1000 {ip}" if os.name == "nt" else f"ping -c 1 -W 1 {ip}")
    return response == 0

def maj_etat_pointeuse(id_pointeuse, etat):
    db = connexion()
    cursor = db.cursor()
    cursor.execute(
        "UPDATE pointeuse SET EstEnligne = %s WHERE idPointeuse = %s",
        (etat, id_pointeuse)
    )
    db.commit()
    cursor.close()
    db.close()

def get_etats_pointeuses(period=60, run_once=True):
    """Vérifie l'état des pointeuses.

    Si `run_once` est True la fonction effectue un seul passage et retourne la liste `etats`.
    Si `run_once` est False, elle boucle indéfiniment en attendant `period` secondes entre chaque vérification.
    """
    while True:
        db = connexion()
        cursor = db.cursor()
        cursor.execute("SELECT idPointeuse, AdresseIp,NomPointeuse,Emplacement,Serie,Model FROM pointeuse")
        pointeuses = cursor.fetchall()
        cursor.close()
        db.close()
        etats = []
        for pointeuse_id, ip, Nom, Localisation, Serie, Modele in pointeuses:
            try:
                etat = "En ligne" if is_pingable(ip) else "Hors ligne"
            except Exception:
                etat = "Hors ligne"
            try:
                maj_etat_pointeuse(pointeuse_id, etat)
            except Exception as e:
                print(f"[WARN] Impossible de mettre à jour l'état en base pour {pointeuse_id} : {e}")

            etats.append({
                "id": pointeuse_id,
                "ip": ip,
                "etat": etat,
                "Nom": Nom,
                "Localisation": Localisation,
                "Serie": Serie,
                "Modele": Modele
            })

        print(f"[INFO] Nouvelle vérification des états.")
        pointeuseenligne = [e['id'] for e in etats if e.get('etat') == 'En ligne']
        print(f"[INFO] Pointeuses en ligne : {pointeuseenligne}")

        # Si on ne veut qu'un seul passage, retourner immédiatement (évite de bloquer l'application)
        if run_once:
            return etats

        # Sinon attendre et recommencer
        print(f"[INFO] Nouvelle vérification des états dans {period} secondes...")
        time.sleep(period)
        

def recuperation_emprientes(period=300):
    while True:
        db = connexion()
        cursor = db.cursor()
        cursor.execute("SELECT idPointeuse, AdresseIp FROM pointeuse")
        pointeuses = cursor.fetchall()
        cursor.close()
        db.close()

        for pointeuse_id, ip in pointeuses:
            print(f"\n📡 Vérification de la pointeuse N°{pointeuse_id} à l'adresse {ip}")
            
            if is_pingable(ip):
                print(f"✅ {ip} est en ligne. Connexion...")

                zk = ZK(ip, port=4370, timeout=10)
                try:
                    conn = zk.connect()
                    conn.disable_device()

                    users = conn.get_users()
                    for user in users:
                        for fid in range(10):
                            template = conn.get_user_template(user.uid, fid)
                            if template and template.size > 0:
                                print(f'Empreinte utilisateur {user.name} (uid {user.uid}) - Finger ID {fid} : {template.size} octets')

                                db2 = connexion()
                                cursor2 = db2.cursor()

                                cursor2.execute("SELECT IDEmploye,IDPointeuse FROM empreintes WHERE IDEmploye=%s AND IDEmpreinte=%s", (user.user_id, fid))
                                if cursor2.fetchone():
                                    print("⚠️ Déjà enregistré.")
                                else:
                                    print(f"Tentative d'insertion : user_id={user.user_id}, name={user.name}, fid={fid}, taille={template.size}")
                                    cursor2.execute("INSERT INTO `empreintes`(`ID_DEVICE`,`IDEmploye`, `Nom`, `IDEmpreinte`, `Empreintes`, `IDPointeuse`) VALUES (%s, %s, %s, %s, %s, %s)",
                                                   (user.uid,user.user_id, user.name, fid, template.template, pointeuse_id))
                                    print("✅ Empreinte enregistrée.")
                                
                                db2.commit()
                                cursor2.close()
                                db2.close()

                    conn.enable_device()
                    conn.disconnect()
                    

                except Exception as e:
                    print(f"❌ Erreur de connexion avec {ip} : {e}")
                except pymysql.MySQLError as e:
                    print(f"❌ Erreur MySQL : {e}")    
            else:
                print(f"❌ {ip} ne répond pas au ping.")
                print("⚠️ Vérifiez la connexion réseau ou l'alimentation de la pointeuse.")
        print(f"[INFO] Nouvelle vérification des empreintes dans {period} secondes...")
        time.sleep(period)

