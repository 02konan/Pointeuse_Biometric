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
                        cursor.execute(insert_sql, (record.user_id, record.timestamp, record.timestamp.date(), ip[1]))
                        db.commit()
                        db.close()

                        last_processed_timestamp = record.timestamp
                        print(f"[NOUVEAU] ID: {record.user_id} | Heure: {record.timestamp}")

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

         
        