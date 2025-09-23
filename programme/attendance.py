from zk import ZK, const
import pymysql
import time
from datetime import datetime, timedelta
import os
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
                new_timestamps = []

                for record in attendances:
                    if last_processed_timestamp is None or record.timestamp > last_processed_timestamp:
                        db = connexion()
                        cursor = db.cursor()

                        # Vérifier combien de pointages déjà dans la journée
                        verification = """
                            SELECT COUNT(*)
                            FROM pointages
                            WHERE IDEmploye = %s
                              AND DATE(date_pointage) = DATE(%s)
                              AND idPointeuse = %s
                        """
                        cursor.execute(verification, (record.user_id, record.timestamp, ip[1]))
                        nb_pointages = cursor.fetchone()[0]

                        jour_semaine = ["Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche"]
                        jour = jour_semaine[record.timestamp.weekday()]

                        if nb_pointages == 0:
                            # Premier pointage -> INSERT
                            insertion = """
                                INSERT IGNORE INTO pointages (IDEmploye, date_pointage, jour_pointage, idPointeuse,Status)
                                VALUES (%s, %s, %s, %s,%s)
                            """
                            cursor.execute(insertion, (record.user_id, record.timestamp, jour, ip[1],"Arrivée enregistrée"))
                            print(f"[NOUVEAU 1er] ID: {record.user_id} | Heure: {record.timestamp}")

                        elif nb_pointages == 1:
                            # Deuxième pointage -> INSERT
                            insertion = """
                                INSERT IGNORE INTO pointages (IDEmploye, date_pointage, jour_pointage, idPointeuse,Status)
                                VALUES (%s, %s, %s, %s,%s)
                            """
                            cursor.execute(insertion, (record.user_id, record.timestamp, jour, ip[1],"Départ enregistré"))
                            print(f"[NOUVEAU 2e] ID: {record.user_id} | Heure: {record.timestamp}")

                        else:
                            # 3e et suivants -> UPDATE uniquement le 2e pointage
                            cursor.execute("""
                                SELECT id
                                FROM pointages
                                WHERE IDEmploye = %s
                                  AND DATE(date_pointage) = DATE(%s)
                                  AND idPointeuse = %s
                                ORDER BY date_pointage ASC
                                LIMIT 1 OFFSET 1
                            """, (record.user_id, record.timestamp, ip[1]))
                            second_pointage = cursor.fetchone()

                            if second_pointage:
                                mise_ajour = """
                                    UPDATE pointages
                                    SET date_pointage = %s , Status=%s
                                    WHERE id = %s
                                """
                                cursor.execute(mise_ajour, (record.timestamp,"Départ enregistré",second_pointage[0],))
                                print(f"[MAJ 2e] ID: {record.user_id} | Nouveau: {record.timestamp}")

                        db.commit()
                        db.close()
                        new_timestamps.append(record.timestamp)

                # Met à jour le dernier timestamp global après avoir tout traité
                if new_timestamps:
                    last_processed_timestamp = max(new_timestamps)

                time.sleep(5)

            except Exception as e:
                print(f"[ERREUR] Problème avec la pointeuse {ip_str}: {e}")

            finally:
                try:
                    conn.enable_device()
                    conn.disconnect()
                except:
                    pass

def programme_attendence():
    while True:
        try:
            with connexion() as conn:
                with conn.cursor() as curseur:
                    sql_insert = """
                    INSERT INTO pointage_programe 
(IDProgramme, IDPointage, Status, arrivee, depart, Duree_initial, Duree_finale)
SELECT 
    pr.IDProgramme,
    ptg.IDPointage,
    CASE 
        WHEN TIMESTAMPDIFF(MINUTE, ptg.arrivee, ptg.depart) >= pr.duree_cours 
            THEN 'Présent'
        ELSE 'Absent'
    END AS Status,
    ptg.arrivee,
    ptg.depart,
    pr.duree_cours,
    TIMEDIFF(ptg.depart, ptg.arrivee) AS duree_finale
FROM Programme pr
JOIN (
    SELECT
        MIN(id) AS IDPointage,
        IDEmploye,
        jour_pointage,
        MIN(TIME(date_pointage)) AS arrivee,
        MAX(TIME(date_pointage)) AS depart
    FROM pointages
    WHERE DATE(date_pointage) = CURDATE()
    GROUP BY IDEmploye, jour_pointage
    HAVING COUNT(*) >= 2
) ptg
ON pr.professeur_code = ptg.IDEmploye
AND pr.jour = ptg.jour_pointage
ON DUPLICATE KEY UPDATE
    Status = VALUES(Status),
    arrivee = VALUES(arrivee),
    depart = VALUES(depart),
    Duree_initial = VALUES(Duree_initial),
    Duree_finale = VALUES(Duree_finale);
                    """

                    curseur.execute(sql_insert)
                    conn.commit()

                    print("[INFO] Mise à jour de pointage_programme effectuée.")

        except pymysql.MySQLError as e:
            print(f"[ERREUR] MySQL: {e}")
        except Exception as e:
            print(f"[ERREUR] Générale: {e}")

def synchronisation_attendance():
    while True:
      programme_attendence()
      time.sleep(60)

def programme_valider(IDemploye, date_pointage,idpointeuse, jour_pointage):
    sql_valider = """insert into pointages (IDEmploye, date_pointage, idPointeuse,jour_pointage)
                     values (%s, %s, %s,%s)"""
    with connexion() as conn:
        try:
            with conn.cursor() as curseur:
                curseur.execute(sql_valider, (IDemploye, date_pointage,idpointeuse,jour_pointage,))
                conn.commit()
                return True
        except pymysql.MySQLError as e:
            print(f"[ERREUR] MySQL: {e}")
            return False
        except Exception as e:
            print(f"[ERREUR] Générale: {e}")
            return False