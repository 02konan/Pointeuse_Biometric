import pymysql

from programme.base_donnee import connexion


class BiometricDeviceMonitor:
    """Lecture de l'état des pointeuses pour le tableau de bord.

    L'état (colonne EstEnligne) est tenu à jour par les fils de surveillance
    de programme/pointeuses.py : le tableau de bord ne pingue plus chaque
    pointeuse à l'affichage, ce qui le bloquait plusieurs secondes quand
    elles étaient hors ligne.
    """

    def __init__(self, connection_factory=None):
        self._connection_factory = (
            connexion if connection_factory is None else connection_factory
        )

    def get_etats_pointeuses(self):
        try:
            with self._connection_factory() as db:
                with db.cursor() as cursor:
                    cursor.execute(
                        "SELECT idPointeuse, AdresseIp, NomPointeuse, Emplacement, Serie, Model, EstEnligne "
                        "FROM pointeuse"
                    )
                    pointeuses = cursor.fetchall()
        except pymysql.MySQLError as e:
            print(f"[ERREUR] Lecture de l'état des pointeuses : {e}")
            return []
        return [
            {
                "id": pointeuse_id,
                "ip": ip,
                "etat": etat if etat in ("En ligne", "Hors ligne") else "Hors ligne",
                "Nom": nom,
                "Localisation": localisation,
                "Serie": serie,
                "Modele": modele,
            }
            for pointeuse_id, ip, nom, localisation, serie, modele, etat in pointeuses
        ]


_service = BiometricDeviceMonitor(connection_factory=connexion)
get_etats_pointeuses = _service.get_etats_pointeuses
