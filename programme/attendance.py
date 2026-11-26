import pymysql
import time
from datetime import datetime, timedelta
from programme.base_donnee import connexion


class AttendanceService:
    def __init__(self, connection_factory=None):
        self._connection_factory = (
            connexion if connection_factory is None else connection_factory
        )

    def programme_attendence(self):
        """Recalcule pointage_programe (présences confrontées à l'emploi du temps)."""
        try:
            with self._connection_factory() as conn:
                with conn.cursor() as curseur:
                    sql_insert = """
                    INSERT INTO pointage_programe
    (IDProgramme, IDPointage, Status, arrivee, depart, Duree_initial, Duree_finale, jour_programme, Date_Programme, section)
    SELECT
        pr.IDProgramme,
        ptg.IDPointage,
        CASE
        WHEN TIMEDIFF(ptg.depart, ptg.arrivee) >= pr.duree_cours
            THEN 'Présent'
        ELSE 'Absent'
        END AS Status,
        ptg.arrivee,
        ptg.depart,
        pr.duree_cours,
        TIMEDIFF(ptg.depart, ptg.arrivee) AS duree_finale,
        ptg.jour_pointage,
        ptg.date_valider,
        pr.section
    FROM Programme pr
    JOIN (
        SELECT
        MIN(id) AS IDPointage,
        IDEmploye,
        jour_pointage,
        MIN(TIME(date_pointage)) AS arrivee,
        MAX(TIME(date_pointage)) AS depart,
        DATE(date_pointage) AS date_valider
        FROM pointages
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
        Duree_finale = VALUES(Duree_finale),
        jour_programme = VALUES(jour_programme),
        Date_Programme = VALUES(Date_Programme),
        section = VALUES(section);
                    """
                    curseur.execute(sql_insert)
                    conn.commit()
                    print("[INFO] Mise à jour de pointage_programme effectuée.")

        except pymysql.MySQLError as e:
            print(f"[ERREUR] MySQL: {e}")
        except Exception as e:
            print(f"[ERREUR] Générale: {e}")

    def synchronisation_attendance(self, period=60):
        # Une passe par minute : l'ancienne boucle tournait sans pause et
        # sollicitait la base en continu.
        while True:
            self.programme_attendence()
            time.sleep(period)

    def programme_valider(self, IDemploye, date_pointage,idpointeuse, jour_pointage,Status):
        sql_valider = """insert into pointages (IDEmploye, date_pointage, idPointeuse,jour_pointage,Status)
                         values (%s, %s, %s, %s, %s)"""
        with self._connection_factory() as conn:
            try:
                with conn.cursor() as curseur:
                    curseur.execute(sql_valider, (IDemploye, date_pointage,idpointeuse,jour_pointage,Status))
                    conn.commit()
                    return True
            except pymysql.MySQLError as e:
                print(f"[ERREUR] MySQL: {e}")
                return False
            except Exception as e:
                print(f"[ERREUR] Générale: {e}")
                return False


_service = AttendanceService(connection_factory=connexion)
programme_attendence = _service.programme_attendence
synchronisation_attendance = _service.synchronisation_attendance
programme_valider = _service.programme_valider
