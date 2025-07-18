from zk import ZK, const
import pymysql
import time
from datetime import datetime, timedelta
import platform
import os
from base_donnee import connexion
ZK_IP = '192.168.1.212'
ZK_PORT = 4370

RECONNECT_DELAY = 5 
last_processed_timestamp = None

def get_db_connection():
    return connexion()
def get_last_pointage_timestamp():
    db = get_db_connection()
    cursor = db.cursor()
    cursor.execute("SELECT MAX(date_pointage) FROM Pointages")
    result = cursor.fetchone()
    db.close()
    return result[0] if result and result[0] else None

def listen_attendance():
    last_processed_timestamp = get_last_pointage_timestamp()
    print(f"[INFO] Dernier pointage déjà traité : {last_processed_timestamp}")

    while True:
        zk = ZK(ZK_IP, port=ZK_PORT, timeout=20)
        try:
            print("[INFO] Connexion au capteur...")
            conn = zk.connect()
            conn.disable_device()
            print("[INFO] Écoute des nouveaux pointages...")

            while True:
                attendances = conn.get_attendance()

                for record in attendances:
                    if last_processed_timestamp is None or record.timestamp > last_processed_timestamp:
                        db = get_db_connection()
                        cursor = db.cursor()

                        # Supprimer les doublons proches (±5s)
                        time_min = record.timestamp - timedelta(seconds=5)
                        time_max = record.timestamp + timedelta(seconds=5)
                        delete_sql = """
                            DELETE FROM pointages 
                            WHERE IDEmploye = %s AND date_pointage BETWEEN %s AND %s
                        """
                        cursor.execute(delete_sql, (record.user_id, time_min, time_max))

                        # Insérer le nouveau pointage
                        insert_sql = """
                            INSERT INTO pointages (IDEmploye, date_pointage) 
                            VALUES (%s, %s)
                        """
                        cursor.execute(insert_sql, (record.user_id, record.timestamp))
                        db.commit()
                        db.close()

                        last_processed_timestamp = record.timestamp
                        retour_pointeur=f"[NOUVEAU] ID: {record.user_id} | Heure: {record.timestamp}"

                time.sleep(2)
                if last_processed_timestamp is not None:
                    print(f"[INFO] Dernier pointage traité : {last_processed_timestamp}")
                else:
                    print("[INFO] Aucun pointage traité jusqu'à présent.")
        except Exception as e:
            print(f"[ERREUR] {e}")
            print(f"[INFO] Reconnexion dans {RECONNECT_DELAY} secondes...")
            time.sleep(RECONNECT_DELAY)

        finally:
            try:
                conn.enable_device()
                conn.disconnect()
                print("[INFO] Déconnecté proprement du capteur.")
            except:
                pass
         
        