from zk import ZK, const
import pymysql
import threading
import time

ZK_IP = '192.168.1.212'
ZK_PORT = 4370
RECONNECT_DELAY = 5  

db = pymysql.connect(host="localhost", user="root", password="", database="ifsm_database")
cursor = db.cursor()

last_processed_timestamp = None

def listen_attendance():
    global last_processed_timestamp
    while True:
        zk = ZK(ZK_IP, port=ZK_PORT, timeout=15)
        try:
            print("[INFO] Connexion au capteur...")
            conn = zk.connect()
            conn.disable_device()
            print("[INFO] Connecté au capteur. Ecoute des pointages...")

            while True:
                attendances = conn.get_attendance()

                for record in attendances:
                    if last_processed_timestamp is None or record.timestamp > last_processed_timestamp:
                        print(f"[Nouveau pointage] User ID: {record.user_id} | Heure: {record.timestamp}")
                        #supprimer les pointage des meme utilisateur a la meme heure mais dans un intervalle de 5 secondes
                        sql = "DELETE FROM empreintes_utilisees WHERE user_id = %s AND heure_pointage BETWEEN %s AND %s"

                        # Insertion dans MySQL
                        sql = "INSERT INTO empreintes_utilisees (user_id, heure_pointage) VALUES (%s, %s)"
                        cursor.execute(sql, (record.user_id, record.timestamp))
                        db.commit()

                        # Mettre à jour le dernier pointage traité
                        last_processed_timestamp = record.timestamp

                time.sleep(2)

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

# --- Démarrage du thread ---
if __name__ == "__main__":
    thread = threading.Thread(target=listen_attendance)
    thread.start()

    while True:
        # Ton serveur principal continue de tourner sans être bloqué
        time.sleep(10)
