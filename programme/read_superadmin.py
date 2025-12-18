import pymysql
from programme.base_donnee import connexion

def read_data_Admin():
    data_base = connexion()
    try:
        with data_base.cursor() as cursor:
            # --- Total employés ---
            sql1 = """
         SELECT COUNT(Distinct matricule) AS total_employes
         FROM employe;
        """

            # --- Présents aujourd’hui ---
            sql2 = """
         SELECT COUNT(*) AS nb_presences
         FROM (
         SELECT p.IDEmploye
         FROM pointages p
         INNER JOIN pointeuse pt ON pt.idPointeuse = p.idPointeuse
         INNER JOIN section s ON s.idPointeuse = pt.idPointeuse
         WHERE DATE(p.date_pointage) = CURRENT_DATE()
         GROUP BY p.IDEmploye
         HAVING COUNT(*) >= 2
         ) AS liste_presences;
         """
            
            # --- Retardataires aujourd’hui ---
            sql3 = """
         SELECT COUNT(*) AS nb_retardataires
         FROM (
         SELECT 
         p.IDEmploye,
         MIN(TIME(p.date_pointage)) AS heure_arrivee_reelle,
         pr.heure_arrivee AS heure_cours
         FROM pointages p
         JOIN Programme pr 
         ON pr.professeur_code = p.IDEmploye
         AND pr.jour = p.jour_pointage
         JOIN section s ON s.idPointeuse = p.idPointeuse
         WHERE p.Status = 'Arrivée enregistrée' 
         AND DATE(p.date_pointage) = CURRENT_DATE()
         AND p.IDEmploye IS NOT NULL
         GROUP BY p.IDEmploye, pr.heure_arrivee
         HAVING heure_arrivee_reelle > pr.heure_arrivee
         ) AS retardataires;
         """

            # --- Absents aujourd’hui ---
            sql4 = """
 SELECT COUNT(*) AS nb_absents
 FROM Programme pr
 LEFT JOIN pointages p 
       ON p.IDEmploye = pr.professeur_code
      AND DATE(p.date_pointage) = CURRENT_DATE()
 WHERE pr.jour =
    CASE DAYOFWEEK(CURRENT_DATE())
        WHEN 2 THEN 'Lundi'
        WHEN 3 THEN 'Mardi'
        WHEN 4 THEN 'Mercredi'
        WHEN 5 THEN 'Jeudi'
        WHEN 6 THEN 'Vendredi'
        WHEN 7 THEN 'Samedi'
        WHEN 1 THEN 'Dimanche'
    END
  AND p.IDEmploye IS NULL;
"""

            # --- Activité récente ---
            sql5 = """
SELECT DISTINCT e.Nom, p.date_pointage, p.Status,s.NomSection
FROM pointages p
JOIN empreintes e ON p.IDEmploye = e.IDEmploye
JOIN pointeuse pt ON pt.idPointeuse = p.idPointeuse
JOIN section s ON s.idPointeuse = pt.idPointeuse
ORDER BY p.date_pointage DESC
LIMIT 8;
"""

            # --- Employés actifs du mois ---
            sql6_mois = """
SELECT COUNT(DISTINCT sous.IDEmploye) AS employes_actifs_mois
FROM (
        SELECT p.IDEmploye
        FROM pointages p
        JOIN empreintes e ON e.IDEmploye = p.IDEmploye
        WHERE YEAR(p.date_pointage) = YEAR(CURRENT_DATE())
            AND MONTH(p.date_pointage) = MONTH(CURRENT_DATE())
        GROUP BY p.IDEmploye, DATE(p.date_pointage)
        HAVING COUNT(p.date_pointage) >= 2
) AS sous;
"""

            # --- Jours travaillés du mois ---
            sql7_mois = """
            SELECT COUNT(*) AS nb_absents_mois
FROM Programme pr
WHERE EXISTS (
        SELECT 1 
        FROM pointages p2
        WHERE p2.jour_pointage = pr.jour
          AND YEAR(p2.date_pointage) = YEAR(CURRENT_DATE())
          AND MONTH(p2.date_pointage) = MONTH(CURRENT_DATE())
    )
    AND NOT EXISTS (
        SELECT 1
        FROM pointages p
        WHERE p.IDEmploye = pr.professeur_code
          AND p.jour_pointage = pr.jour
          AND YEAR(p.date_pointage) = YEAR(CURRENT_DATE())
          AND MONTH(p.date_pointage) = MONTH(CURRENT_DATE())
    );
"""

            # --- Retardataires du mois ---
            sql8_mois = """
SELECT COUNT(*) AS employes_retard_mois
FROM (
    SELECT
        p.IDEmploye,
        DATE(p.date_pointage) AS jour,
        MIN(TIME(p.date_pointage)) AS heure_arrivee_reelle,
        pr.heure_arrivee AS heure_programme
    FROM pointages p
    JOIN Programme pr
        ON pr.professeur_code = p.IDEmploye
       AND pr.jour = p.jour_pointage
    WHERE 
        YEAR(p.date_pointage) = YEAR(CURRENT_DATE())
        AND MONTH(p.date_pointage) = MONTH(CURRENT_DATE())
        AND p.Status = 'Arrivée enregistrée'
        AND p.IDEmploye IS NOT NULL
    GROUP BY 
        p.IDEmploye,
        DATE(p.date_pointage),
        pr.heure_arrivee
    HAVING 
        heure_arrivee_reelle > heure_programme
) AS retards;

"""
           
            # --- Exécution ---
            cursor.execute(sql1)
            total_employes = cursor.fetchone()[0]

            cursor.execute(sql2)
            total_Presents = cursor.fetchone()[0]

            cursor.execute(sql3)
            total_retard = cursor.fetchone()[0]

            cursor.execute(sql4)
            total_absents = cursor.fetchone()[0]

            cursor.execute(sql5)
            activite_recentes = cursor.fetchall()

            cursor.execute(sql6_mois)
            employes_actifs_mois = cursor.fetchone()[0]

            cursor.execute(sql7_mois)
            jours_travailles_mois = cursor.fetchone()[0]

            cursor.execute(sql8_mois)
            employes_retard_mois = cursor.fetchone()[0]

            return (
                total_employes,
                total_Presents,
                total_retard,
                activite_recentes,
                total_absents,
                employes_actifs_mois,
                jours_travailles_mois,
                employes_retard_mois
            )

    except Exception as e:
        print("Erreur lors de la lecture des données du tableau de bord (Admin) :", e)
        return None
    except pymysql.MySQLError as e:
        print("Erreur lors de la lecture des données sql :", e)
        return None

def read_admin_presence():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql="""SELECT
    pr.professeur_code,
    pr.professeur_nom,
    arrivee,
    depart,
    p.jour_pointage,
    DATE(p.date_pointage) AS date_pointage,
    Duree_initial,
    pp.Status,
    pr.heure_arrivee,
    TIMEDIFF(depart,arrivee) AS temps_presence
FROM pointage_programe pp
JOIN Programme pr ON pr.IDProgramme = pp.IDProgramme
JOIN pointages p ON pp.IDPointage = p.id
GROUP BY pr.professeur_code, pr.professeur_nom, DATE(p.date_pointage)
ORDER BY p.date_pointage DESC, temps_presence;
                      """
                cursor.execute(sql)
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

def pointage_admin_invalid():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = """SELECT 
    p.professeur_code,
    p.professeur_nom,
    DATE(ptg.date_pointage),
    po.idPointeuse,
    TIME(ptg.date_pointage) AS heure_pointage,
    p.heure_arrivee,
    p.heure_depart,
    p.duree_cours,
    ptg.jour_pointage
    FROM Programme p
    INNER JOIN pointages ptg ON p.professeur_code = ptg.IDEmploye
    AND DATE(ptg.date_pointage) =CURDATE() AND ptg.jour_pointage=p.jour
    INNER JOIN pointeuse po ON po.idPointeuse = ptg.idPointeuse
    INNER JOIN section s ON s.idPointeuse = po.idPointeuse
    GROUP BY 
    p.professeur_code,
    p.professeur_nom
    HAVING COUNT(DISTINCT ptg.id)=1
    ORDER BY p.professeur_code
                """
                cursor.execute(sql)
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
        return []

def generer_presence_admin(date_debut, date_fin,idemployee):
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = """
                SELECT 
    pr.professeur_nom,jour_pointage,
    DATE(p.date_pointage) AS date_pointage,
    s.NomSection as Section,
    SEC_TO_TIME(SUM(TIME_TO_SEC(pr.duree_cours))) AS total_heures_cours,
    SEC_TO_TIME(SUM(TIME_TO_SEC(pp.Duree_finale))) AS total_heures_effectuer,
    SEC_TO_TIME(SUM(TIME_TO_SEC(pp.Duree_finale)) - SUM(TIME_TO_SEC(pr.duree_cours))) AS ecart,
    CASE
        WHEN SUM(TIME_TO_SEC(pp.Duree_finale)) = SUM(TIME_TO_SEC(pr.duree_cours)) THEN 'Complet'
        WHEN SUM(TIME_TO_SEC(pp.Duree_finale)) < SUM(TIME_TO_SEC(pr.duree_cours)) THEN 'Manque du temps'
        WHEN SUM(TIME_TO_SEC(pp.Duree_finale)) > SUM(TIME_TO_SEC(pr.duree_cours)) THEN 'Excédent'
        ELSE 'Non défini'
    END AS observation
    FROM pointage_programe pp
    JOIN Programme pr ON pr.IDProgramme = pp.IDProgramme
    JOIN pointages p ON p.id = pp.IDPointage
    JOIN pointeuse pt on pt.idPointeuse=p.IDPointeuse
    JOIN section s on s.idPointeuse=pt.idPointeuse
    WHERE DATE(p.date_pointage) BETWEEN %s AND %s AND p.IDEmploye=%s
    GROUP BY pr.professeur_nom, jour_pointage, p.date_pointage
    ORDER BY p.date_pointage;
                """
                cursor.execute(sql, (date_debut, date_fin, idemployee))
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)

def data_chatjs():
    try:
        data_base = connexion()
        with data_base.cursor() as cursor:
            # --- Présents aujourd’hui ---
            sql2 = """
 WITH jours AS (
    SELECT CURDATE() - INTERVAL n DAY AS jour
    FROM (
        SELECT 0 AS n UNION SELECT 1 UNION SELECT 2 UNION SELECT 3 
        UNION SELECT 4 UNION SELECT 5 UNION SELECT 6
    ) AS nums
)
SELECT 
    CASE DAYOFWEEK(j.jour)
        WHEN 1 THEN 'Dimanche'
        WHEN 2 THEN 'Lundi'
        WHEN 3 THEN 'Mardi'
        WHEN 4 THEN 'Mercredi'
        WHEN 5 THEN 'Jeudi'
        WHEN 6 THEN 'Vendredi'
        WHEN 7 THEN 'Samedi'
    END AS jour_francais,
    COUNT(lp.IDEmploye) AS nb_presences
FROM jours j
LEFT JOIN (
    SELECT 
        IDEmploye, 
        DATE(date_pointage) AS jour_pointage
    FROM pointages
    GROUP BY IDEmploye, DATE(date_pointage)
    HAVING COUNT(*) >= 2
) AS lp
ON lp.jour_pointage = j.jour
GROUP BY j.jour
ORDER BY 
    CASE DAYOFWEEK(j.jour)
        WHEN 2 THEN 1
        WHEN 3 THEN 2
        WHEN 4 THEN 3
        WHEN 5 THEN 4
        WHEN 6 THEN 5
        WHEN 7 THEN 6
        WHEN 1 THEN 7
    END;
"""

            # --- Retardataires aujourd’hui ---
            sql3 = """WITH jours AS (
    SELECT CURDATE() - INTERVAL n DAY AS jour
    FROM (
        SELECT 0 AS n UNION SELECT 1 UNION SELECT 2 UNION SELECT 3
        UNION SELECT 4 UNION SELECT 5 UNION SELECT 6
    ) AS nums
),
retardataires AS (
    SELECT 
        p.IDEmploye,
        DATE(p.date_pointage) AS jour_pointage,
        MIN(TIME(p.date_pointage)) AS heure_arrivee_reelle,
        pr.heure_arrivee AS heure_cours
    FROM pointages p
    JOIN Programme pr 
        ON pr.professeur_code = p.IDEmploye
        AND pr.jour = CASE DAYOFWEEK(p.date_pointage)
                        WHEN 1 THEN 'Dimanche'
                        WHEN 2 THEN 'Lundi'
                        WHEN 3 THEN 'Mardi'
                        WHEN 4 THEN 'Mercredi'
                        WHEN 5 THEN 'Jeudi'
                        WHEN 6 THEN 'Vendredi'
                        WHEN 7 THEN 'Samedi'
                      END
    WHERE p.Status = 'Arrivée enregistrée'
      AND p.date_pointage >= CURDATE() - INTERVAL 6 DAY
    GROUP BY p.IDEmploye, DATE(p.date_pointage), pr.heure_arrivee
    HAVING heure_arrivee_reelle > heure_cours
)
SELECT 
    CASE DAYOFWEEK(j.jour)
        WHEN 1 THEN 'Dimanche'
        WHEN 2 THEN 'Lundi'
        WHEN 3 THEN 'Mardi'
        WHEN 4 THEN 'Mercredi'
        WHEN 5 THEN 'Jeudi'
        WHEN 6 THEN 'Vendredi'
        WHEN 7 THEN 'Samedi'
    END AS jour_francais,
    COUNT(r.IDEmploye) AS nb_retard
FROM jours j
LEFT JOIN retardataires r 
       ON r.jour_pointage = j.jour
GROUP BY j.jour
ORDER BY 
    CASE DAYOFWEEK(j.jour)
        WHEN 2 THEN 1  -- Lundi
        WHEN 3 THEN 2  -- Mardi
        WHEN 4 THEN 3  -- Mercredi
        WHEN 5 THEN 4  -- Jeudi
        WHEN 6 THEN 5  -- Vendredi
        WHEN 7 THEN 6  -- Samedi
        WHEN 1 THEN 7  -- Dimanche
    END;

"""

            # --- Absents aujourd’hui ---
            sql4 = """
 SELECT 
    CASE DAYOFWEEK(j.jour)
        WHEN 1 THEN 'Dimanche'
        WHEN 2 THEN 'Lundi'
        WHEN 3 THEN 'Mardi'
        WHEN 4 THEN 'Mercredi'
        WHEN 5 THEN 'Jeudi'
        WHEN 6 THEN 'Vendredi'
        WHEN 7 THEN 'Samedi'
    END AS jour_francais,
    COUNT(prc.IDEmploye) AS nb_absents
 FROM (
    SELECT CURDATE() - INTERVAL n DAY AS jour
    FROM (
        SELECT 0 AS n UNION SELECT 1 UNION SELECT 2 UNION SELECT 3 UNION
        SELECT 4 UNION SELECT 5 UNION SELECT 6
    ) AS nums
  ) AS j
JOIN (
    SELECT pr.professeur_code AS IDEmploye, pr.jour AS jour_cours
    FROM Programme pr
) AS prc
  ON prc.jour_cours = CASE DAYOFWEEK(j.jour)
                            WHEN 1 THEN 'Dimanche'
                            WHEN 2 THEN 'Lundi'
                            WHEN 3 THEN 'Mardi'
                            WHEN 4 THEN 'Mercredi'
                            WHEN 5 THEN 'Jeudi'
                            WHEN 6 THEN 'Vendredi'
                            WHEN 7 THEN 'Samedi'
                        END
LEFT JOIN pointages p 
       ON p.IDEmploye = prc.IDEmploye
      AND DATE(p.date_pointage) = j.jour
WHERE p.IDEmploye IS NULL
GROUP BY j.jour
ORDER BY 
    CASE DAYOFWEEK(j.jour)
        WHEN 2 THEN 1
        WHEN 3 THEN 2
        WHEN 4 THEN 3
        WHEN 5 THEN 4
        WHEN 6 THEN 5
        WHEN 7 THEN 6
        WHEN 1 THEN 7
    END;
"""
            
            cursor.execute(sql2)
            chartjs_Presents = cursor.fetchall()

            cursor.execute(sql3)
            chartjs_retard = cursor.fetchall()

            cursor.execute(sql4)
            chartjs_absents = cursor.fetchall()

            return chartjs_Presents, chartjs_retard, chartjs_absents
    except Exception as e:
        print("Erreur lors de la lecture des données pour le graphique :", e)
        return None, None, None

def read_pointage():
     try:
         with connexion() as conn:
             with conn.cursor() as cusor:
                 sql= """
             SELECT DISTINCT e.Nom, p.date_pointage, p.Status,s.NomSection
             FROM pointages p
             JOIN empreintes e ON p.IDEmploye = e.IDEmploye
             JOIN pointeuse pt ON pt.idPointeuse = p.idPointeuse
             JOIN section s ON s.idPointeuse = pt.idPointeuse
             ORDER BY p.date_pointage DESC"""
                 cusor.execute(sql)
                 return cusor.fetchall()
     except pymysql.MySQLError as e:
         print("Erreur Mysql:",e)
     except Exception as e:
         print("Erreur Generele:",e)

def historique_data():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql="""
             SELECT DISTINCT e.Nom, date(p.date_pointage) as date_pointage,time(p.date_pointage) as heure_pointage, p.Status,s.NomSection
             FROM pointages p
             JOIN empreintes e ON p.IDEmploye = e.IDEmploye
             JOIN pointeuse pt ON pt.idPointeuse = p.idPointeuse
             JOIN section s ON s.idPointeuse = pt.idPointeuse
             ORDER BY p.date_pointage DESC
            """
                cursor.execute(sql)
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)
                
def read_raports():
    try:
        with connexion() as conn:
            with conn.cursor() as cursor:
                sql = """
                SELECT r.fichier, u.nom
                FROM rapports r
                JOIN utilisateurs u ON r.id_utilisateur = u.id
                """
                cursor.execute(sql)
                return cursor.fetchall()
    except pymysql.MySQLError as e:
        print("Erreur MySQL :", e)
    except Exception as e:
        print("Erreur générale :", e)