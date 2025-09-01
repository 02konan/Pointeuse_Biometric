from zk import ZK, const
import pymysql
import time
from datetime import datetime, timedelta
import platform
import os
import locale
from programme.base_donnee import connexion

RECONNECT_DELAY = 5 
last_processed_timestamp = None
def is_pingable(ip):
    response = os.system(f"ping -n 1 -w 1000 {ip}" if os.name == "nt" else f"ping -c 1 -W 1 {ip}")
    return response == 0

def dernier_pointage():
    db = connexion()
    cursor = db.cursor()
    cursor.execute("SELECT MAX(date_pointage) FROM pointages")
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

def programme_attendence():
    sql_pointage = """
        SELECT 
          ID,
          IDEmploye,
          jour_pointage,
          MIN(TIME(date_pointage)) AS arrivee,
          MAX(TIME(date_pointage)) AS depart,
          TIMESTAMPDIFF(
              MINUTE, 
              MIN(date_pointage), 
              MAX(date_pointage)
          ) AS duree_minutes
        FROM pointages
        GROUP BY IDEmploye, DATE(date_pointage)
        HAVING COUNT(*) >= 2
        ORDER BY jour_pointage DESC;
    """
    with connexion() as conn:
        try:
            with conn.cursor() as curseur:
                curseur.execute(sql_pointage)
                result = curseur.fetchall()

                for pointage in result:
                    id_pointage   = pointage[0]
                    id_Employe    = pointage[1]
                    jour_pointage = pointage[2]
                    arrivee       = pointage[3]
                    depart        = pointage[4]
                    duree_minutes = pointage[5]

                    sql_programme = """
                        SELECT IDProgramme, professeur_id, professeur_code, 
                               jour, duree_cours
                        FROM Programme
                    """
                    curseur.execute(sql_programme)
                    results_programme = curseur.fetchall()

                    for programme in results_programme:
                        id_programme    = programme[0]
                        professeur_code = programme[2]
                        jour            = programme[3]
                        duree_cours     = programme[4]

                        if jour == jour_pointage and id_Employe == professeur_code:
                            if duree_minutes >= duree_cours:
                                statut = "Présent"
                            else:
                                statut = "Absent"
                            sql_update = """
                                UPDATE pointage_programe
                                SET Status = %s,
                                    arrivee = %s,
                                    depart = %s,
                                    Duree_initial = %s,
                                    Duree_finale = %s
                                WHERE IDProgramme = %s AND IDPointage = %s
                            """
                            ligne = curseur.execute(sql_update, (
                                statut, arrivee, depart, duree_cours, duree_minutes, id_programme, id_pointage
                            ))

                            # --- Si aucun UPDATE => INSERT ---
                            if ligne == 0:
                                insert_Programme_pointage = """
                                    INSERT INTO pointage_programe 
                                        (IDProgramme, IDPointage, Status, arrivee, depart, Duree_initial, Duree_finale)
                                    VALUES (%s, %s, %s, %s, %s, %s, %s)
                                """
                                curseur.execute(insert_Programme_pointage, (
                                    id_programme, id_pointage, statut, arrivee, depart, duree_cours, duree_minutes
                                ))

                conn.commit()
                return True

        except pymysql.MySQLError as e:
            print(f"[ERREUR] MySQL: {e}")
            return False
        except Exception as e:
            print(f"[ERREUR] Générale: {e}")
            return False

# def synchro_attendance(periode=None):
#     while True:
#         listen_attendance()
#         if periode:
#             time.sleep(periode)
#         programme_attendence()
#         time.sleep(periode)