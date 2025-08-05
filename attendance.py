from zk import ZK, const
import pymysql
import time
from datetime import datetime, timedelta
import platform
import os
from base_donnee import connexion

RECONNECT_DELAY = 5 
last_processed_timestamp = None
def is_pingable(ip):
    response = os.system(f"ping -n 1 -w 1000 {ip}" if os.name == "nt" else f"ping -c 1 -W 1 {ip}")
    return response == 0


def dernier_pointage():
    db = connexion()
    cursor = db.cursor()
    cursor.execute("SELECT MAX(date_pointage) FROM Pointages")
    result = cursor.fetchone()
    db.close()
    return result[0] if result and result[0] else None

def get_pointeuses():
    db = connexion()
    cursor = db.cursor()
    cursor.execute("SELECT Adresseip,idPointeuse FROM pointeuse")
    result = cursor.fetchall()
    db.close()
    return result

def listen_attendance():
    global last_processed_timestamp
    last_processed_timestamp = dernier_pointage()
    print(f"[INFO] Dernier pointage déjà traité : {last_processed_timestamp}")

    while True:
        pointeuses = get_pointeuses()
        for ip in pointeuses:
            ip_str = ip[0]
            if not is_pingable(ip_str):
                print(f"[ERREUR] Impossible de joindre la pointeuse {ip_str}. Nouvel essai dans {RECONNECT_DELAY} secondes...")
                continue

            zk = ZK(ip_str, port=int(4370), timeout=20)
            try:
                print(f"[INFO] Connexion à la pointeuse {ip_str} (ID {ip[1]})...")
                conn = zk.connect()
                conn.disable_device()
                print(f"[INFO] Écoute des nouveaux pointages sur {ip_str}...")

                attendances = conn.get_attendance()
                for record in attendances:
                    if last_processed_timestamp is None or record.timestamp > last_processed_timestamp:
                        db = connexion()
                        cursor = db.cursor()

                        time_min = record.timestamp - timedelta(seconds=5)
                        time_max = record.timestamp + timedelta(seconds=5)
                        delete_sql = """
                            DELETE FROM pointages 
                            WHERE IDEmploye = %s AND date_pointage BETWEEN %s AND %s
                        """
                        cursor.execute(delete_sql, (record.user_id, time_min, time_max))

                        insert_sql = """
                            INSERT INTO pointages (IDEmploye, date_pointage, jour_pointage,idPointeuse) 
                            VALUES (%s, %s, %s, %s)
                        """
                        date_str=record.timestamp
                        
                        jour_semaine=["Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi","Dimanche"]
                        jour=jour_semaine[date_str.weekday()]
                        cursor.execute(insert_sql, (record.user_id, date_str,jour, ip[1]))
                        db.commit()
                        db.close()

                        last_processed_timestamp = record.timestamp
                        print(f"[NOUVEAU] ID: {record.user_id} | Heure: {record.timestamp} | jour: {jour}")

                time.sleep(5)

            except Exception as e:
                print(f"[ERREUR] {e}")
                print(f"[INFO] Reconnexion à {ip_str} dans {RECONNECT_DELAY} secondes...")
                time.sleep(RECONNECT_DELAY)

            finally:
                try:
                    conn.enable_device()
                    conn.disconnect()
                    print(f"[INFO] Déconnecté proprement de la pointeuse {ip_str}.")
                except:
                    pass

def programme_attendance():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                pointage_sql="SELECT id,IDEmploye,jour_pointage from pointages"
                cursor.execute(pointage_sql)
                pointage_programme=cursor.fetchall()
                for pointage in pointage_programme:
                    id_programme=pointage[0]
                    ID_Employe=pointage[1]
                    jour=pointage[2]
                    Programme_sql = """
                        SELECT `IDProgramme`, `professeur_id`, `professeur_nom`, `jour`, `heure_arrivee`, `heure_depart`, `duree_cours` 
                        FROM Programme 
                        WHERE professeur_id = %s AND jour = %s
                    """
                    cursor.execute(Programme_sql,(ID_Employe,jour))
                    programme_verifie=cursor.fetchall()
                    for programme in programme_verifie:
                        IDProgramme=programme[0]
                        if programme:
                           insert_sql = """
                                INSERT INTO pointage_programe (IDProgramme, IDPointage, EstValider) 
                                VALUES (%s, %s, %s)
                            """
                           cursor.execute(insert_sql, (IDProgramme, id_programme, 1))
                           print(f"[INFO] Pointage pour le programme {IDProgramme} inséré avec succès.")
                        else:
                            print(f"[INFO] Aucun programme trouvé pour l'employé {ID_Employe} le {jour}.")
                conn.commit()
    except pymysql.MySQLError as e:
        print(f"[ERREUR] Erreur MySQL : {e}")
    except Exception as e:
        print(f"[ERREUR] Erreur : {e}")