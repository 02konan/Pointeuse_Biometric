import pymysql
from programme.base_donnee import connexion

def read_data_Admin():
    data_base = connexion()
    try:
        with data_base.cursor() as cursor:
            # --- Total employés ---
            sql1 = """
 SELECT COUNT(IDEmploye) AS total_employes
 FROM empreintes;
 """

            # --- Présents aujourd’hui ---
            sql2 = """SELECT COUNT(*) AS nb_presences
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
    SELECT p.IDEmploye
    FROM pointages p
    WHERE DATE(p.date_pointage) = CURRENT_DATE()
      AND p.IDEmploye IS NOT NULL
    GROUP BY p.IDEmploye
    HAVING MIN(TIME(p.date_pointage)) > '08:00:00'
) AS retardataires;
"""

            # --- Absents aujourd’hui ---
            sql4 = """
SELECT COUNT(*) AS nb_absents
FROM empreintes e
LEFT JOIN pointages p 
       ON p.IDEmploye = e.IDEmploye 
      AND DATE(p.date_pointage) = CURRENT_DATE()
WHERE p.IDEmploye IS NULL;
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
SELECT COUNT(DISTINCT DATE(p.date_pointage)) AS jours_travailles
FROM pointages p
WHERE YEAR(p.date_pointage) = YEAR(CURRENT_DATE())
  AND MONTH(p.date_pointage) = MONTH(CURRENT_DATE());
"""

            # --- Retardataires du mois ---
            sql8_mois = """
SELECT COUNT(DISTINCT p.IDEmploye) AS employes_retard_mois
FROM pointages p
WHERE YEAR(p.date_pointage) = YEAR(CURRENT_DATE())
  AND MONTH(p.date_pointage) = MONTH(CURRENT_DATE())
  AND TIME(p.date_pointage) > '08:30:00'
  AND NOT EXISTS (
      SELECT 1
      FROM pointages p2
      WHERE p2.IDEmploye = p.IDEmploye
        AND DATE(p2.date_pointage) = DATE(p.date_pointage)
        AND TIME(p2.date_pointage) <= '08:30:00'
  );
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
    SELECT p.IDEmploye, DATE(p.date_pointage) AS jour_pointage
    FROM pointages p
    JOIN empreintes e ON e.IDEmploye = p.IDEmploye
    GROUP BY p.IDEmploye, DATE(p.date_pointage)
    HAVING COUNT(p.IDEmploye) >= 2
) AS lp
ON lp.jour_pointage = j.jour
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

            # --- Retardataires aujourd’hui ---
            sql3 = """
WITH jours AS (
    SELECT CURDATE() - INTERVAL n DAY AS jour
    FROM (
        SELECT 0 AS n UNION SELECT 1 UNION SELECT 2 UNION SELECT 3
        UNION SELECT 4 UNION SELECT 5 UNION SELECT 6
    ) AS nums
),
retardataires AS (
    SELECT p.IDEmploye, DATE(p.date_pointage) AS jour_pointage
    FROM pointages p
    WHERE p.date_pointage >= CURDATE() - INTERVAL 6 DAY
    GROUP BY p.IDEmploye, DATE(p.date_pointage)
    HAVING MIN(TIME(p.date_pointage)) > '08:00:00'
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
WITH jours AS (
    SELECT CURDATE() - INTERVAL n DAY AS jour
    FROM (
        SELECT 0 AS n UNION SELECT 1 UNION SELECT 2 UNION SELECT 3 UNION
        SELECT 4 UNION SELECT 5 UNION SELECT 6
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
    COUNT(e.IDEmploye) AS nb_absents
FROM jours j
CROSS JOIN empreintes e
LEFT JOIN pointages p 
       ON p.IDEmploye = e.IDEmploye
      AND DATE(p.date_pointage) = j.jour
WHERE p.IDEmploye IS NULL
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
             SELECT DISTINCT e.Nom, p.date_pointage, p.Status,s.NomSection
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