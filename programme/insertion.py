import pymysql
import time
import os
from zk import ZK, const
from programme.base_donnee import connexion

def is_pingable(ip):
    response = os.system(f"ping -n 1 -w 1000 {ip}" if os.name == "nt" else f"ping -c 1 -W 1 {ip}")
    return response == 0

def get_pointeuses():
    db = connexion()
    cursor = db.cursor()
    cursor.execute("SELECT Adresseip, idPointeuse FROM pointeuse")
    result = cursor.fetchall()
    db.close()
    return result

def get_employes():
    try:
        with connexion() as db:
            cursor = db.cursor()
            cursor.execute("SELECT professeur_code, professeur_nom FROM programme")  # <-- corrige ici
            return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("❌ Erreur lors de la récupération des employés :", e)
        return []

def insertion_():
    while True:
        pointeuse=get_pointeuses()
        for pointeuse in pointeuse:
            pointeuse_ip = pointeuse[0]
            if not is_pingable(pointeuse_ip):
                print(f"[ERREUR] Impossible de joindre la pointeuse {pointeuse_ip}. Nouvel essai dans 5 secondes...")
                continue
            zk = ZK(pointeuse_ip, port=4370, timeout=20)
        try:
            print(f"[INFO] Connexion à la pointeuse {pointeuse_ip}...")
            conn = zk.connect()
            conn.disable_device()
            print(f"[INFO] Connexion réussie à la pointeuse {pointeuse_ip}.")

            employes = get_employes()
            for emp in employes:
                code, nom = emp
                try:
                    conn.set_user(code, nom)
                    print(f"[INFO] Employé {nom} (Code: {code}) inséré avec succès.")
                except Exception as e:
                    print(f"[ERREUR] Échec de l'insertion pour {nom} (Code: {code}) : {e}")

            conn.enable_device()
        except Exception as e:
            print(f"[ERREUR] Échec de connexion à la pointeuse {pointeuse_ip} : {e}")
        finally:
            zk.disconnect()
        time.sleep(5)   
        