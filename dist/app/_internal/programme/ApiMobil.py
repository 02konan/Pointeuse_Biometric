from programme.base_donnee import connexion
from flask import jsonify


class LegacyMobileService:
    def __init__(self, connection_factory=None):
        self._connection_factory = (
            connexion if connection_factory is None else connection_factory
        )

    def api_programme_route(self, matricule):
        try:
            conn = self._connection_factory()
            cursor = conn.cursor()
            query = """
                SELECT professeur_code,jour, type, heure_arrivee, heure_depart, duree_cours,professeur_nom, Matiere
                FROM Programme
                WHERE professeur_code = %s
                ORDER BY jour DESC
            """
            cursor.execute(query, (matricule,))
            rows = cursor.fetchall()

            programmes = []
            for row in rows:
                programmes.append({
                    "professeur_code": row[0],
                    "journee": str(row[1]),
                    "EmploiDuTmp": row[2],
                    "heure_arrivee": str(row[3]),
                    "heure_depart": str(row[4]),
                    "duree_cours": str(row[5]),
                    "professeur_nom": str(row[6]),
                    "Matiere": str(row[7]),
                })

            return jsonify(programmes)

        except Exception as e:
            return jsonify({"error": str(e)}), 500

    def get_pointages(self, matricule):
        try:
            conn = self._connection_factory()
            cursor = conn.cursor()
            query = """
                SELECT
        DATE(p.date_pointage) AS date_pointage,
        MIN(TIME(p.date_pointage)) AS heure_entree,
        MAX(TIME(p.date_pointage)) AS heure_sortie,
        e.Nom AS Nom_Prenom
        FROM
        pointages p
        JOIN
        empreintes e ON e.IDEmploye = p.IDEmploye
        WHERE
        e.IDEmploye =%s
        AND DATE(p.date_pointage) = CURRENT_DATE()
        GROUP BY
        DATE(p.date_pointage), e.Nom
        ORDER BY
        date_pointage DESC;

            """
            cursor.execute(query, (matricule,))
            rows = cursor.fetchall()

            # Convertir manuellement en liste de dictionnaires
            result = []
            for row in rows:
                result.append({
                    "date_pointage": str(row[0]),
                    "heure_entree": str(row[1]),
                    "heure_sortie": str(row[2]),
                    "Nom_Prenom": str(row[3])
                })

            return jsonify(result)

        except Exception as e:
            return jsonify({"error": str(e)}), 500


_service = LegacyMobileService(connection_factory=connexion)
api_programme_route = _service.api_programme_route
get_pointages = _service.get_pointages
