import threading
import time
import unittest
from datetime import datetime
from types import SimpleNamespace
from unittest.mock import MagicMock, patch

from programme import pointeuses
from programme.pointeuses import SuperviseurPointeuses, SurveillancePointeuse


def fausse_base(curseur=None):
    curseur = curseur or MagicMock()
    db = MagicMock()
    db.__enter__.return_value = db
    db.cursor.return_value.__enter__.return_value = curseur
    return MagicMock(return_value=db), curseur


def fausse_pointeuse(pointages=()):
    conn = MagicMock()
    conn.get_attendance.return_value = list(pointages)
    conn.get_users.return_value = []
    conn.get_templates.return_value = []
    return conn


class EtatEtReconnexionTests(unittest.TestCase):
    def setUp(self):
        self.fabrique, self.curseur = fausse_base()
        self.curseur.fetchall.return_value = []
        self.curseur.fetchone.return_value = (None,)
        self.surveillance = SurveillancePointeuse(1, "192.168.1.100", self.fabrique)

    def etats_ecrits(self):
        return [
            appel.args[1][0]
            for appel in self.curseur.execute.call_args_list
            if appel.args[0].startswith("UPDATE pointeuse")
        ]

    def test_hors_ligne_puis_reconnectee(self):
        with patch.object(pointeuses, "ouvrir", side_effect=OSError("timed out")):
            self.assertFalse(self.surveillance._cycle())
            self.assertFalse(self.surveillance._cycle())
        with patch.object(pointeuses, "ouvrir", return_value=fausse_pointeuse()):
            self.assertTrue(self.surveillance._cycle())

        # L'état n'est écrit qu'à chaque changement, pas à chaque essai.
        self.assertEqual(self.etats_ecrits(), ["Hors ligne", "En ligne"])
        self.assertTrue(self.surveillance.en_ligne)

    def test_une_panne_de_base_ne_rend_pas_la_pointeuse_hors_ligne(self):
        import pymysql

        conn = fausse_pointeuse([SimpleNamespace(user_id="P1", timestamp=datetime(2026, 11, 26, 8))])
        self.surveillance._recuperer_pointages = MagicMock(side_effect=pymysql.MySQLError("perdu"))
        with patch.object(pointeuses, "ouvrir", return_value=conn):
            self.assertTrue(self.surveillance._cycle())
        self.assertTrue(self.surveillance.en_ligne)
        conn.disconnect.assert_called_once()

    def test_le_fil_survit_a_une_erreur_inattendue(self):
        appels = []

        def cycle():
            appels.append(1)
            if len(appels) == 3:
                self.surveillance.arreter()
            raise RuntimeError("imprévu")

        self.surveillance._cycle = cycle
        with patch.object(pointeuses, "ATTENTE_MIN", 0), patch.object(pointeuses, "ATTENTE_MAX", 0):
            self.surveillance.run()
        self.assertEqual(len(appels), 3)


class ConnexionsSimultaneesTests(unittest.TestCase):
    def test_deux_acces_a_la_meme_pointeuse_ne_se_chevauchent_pas(self):
        fabrique, curseur = fausse_base()
        curseur.fetchone.return_value = (None,)
        curseur.fetchall.return_value = []
        en_cours, maximum = [0], [0]
        garde = threading.Lock()

        def ouvrir_lentement(ip, timeout=None):
            with garde:
                en_cours[0] += 1
                maximum[0] = max(maximum[0], en_cours[0])
            time.sleep(0.05)
            conn = fausse_pointeuse()
            conn.disconnect.side_effect = lambda: en_cours.__setitem__(0, en_cours[0] - 1)
            return conn

        a = SurveillancePointeuse(1, "192.168.1.150", fabrique)
        b = SurveillancePointeuse(2, "192.168.1.150", fabrique)  # même IP
        with patch.object(pointeuses, "ouvrir", side_effect=ouvrir_lentement):
            fils = [threading.Thread(target=s._cycle) for s in (a, b, a, b)]
            for f in fils:
                f.start()
            for f in fils:
                f.join()
        self.assertEqual(maximum[0], 1)


class PointagesTests(unittest.TestCase):
    def test_dernier_pointage_propre_a_chaque_pointeuse(self):
        fabrique, curseur = fausse_base()
        # MAX(date_pointage) de CETTE pointeuse, puis 0 puis 1 pointage déjà en base.
        curseur.fetchone.side_effect = [(datetime(2026, 11, 26, 7, 0),), (0,), (1,)]
        surveillance = SurveillancePointeuse(5, "192.168.1.204", fabrique)
        conn = fausse_pointeuse([
            SimpleNamespace(user_id="P1", timestamp=datetime(2026, 11, 26, 6, 0)),   # déjà traité
            SimpleNamespace(user_id="P1", timestamp=datetime(2026, 11, 26, 8, 0)),
            SimpleNamespace(user_id="P1", timestamp=datetime(2026, 11, 26, 16, 0)),
        ])

        surveillance._recuperer_pointages(conn)

        requete_max = curseur.execute.call_args_list[0]
        self.assertIn("WHERE idPointeuse = %s", requete_max.args[0])
        self.assertEqual(requete_max.args[1], (5,))
        insertions = [a.args[1] for a in curseur.execute.call_args_list if "INSERT IGNORE" in a.args[0]]
        self.assertEqual(
            [(i[1].hour, i[2], i[3], i[4]) for i in insertions],
            [(8, "Jeudi", 5, "Arrivée enregistrée"), (16, "Jeudi", 5, "Départ enregistré")],
        )


class SuperviseurTests(unittest.TestCase):
    def test_suit_les_ajouts_changements_et_suppressions(self):
        fabrique, curseur = fausse_base()
        creees = []

        def fabrique_surveillance(id_p, ip, _fabrique):
            s = MagicMock(ip=ip)
            s.is_alive.return_value = True
            creees.append((id_p, ip))
            return s

        superviseur = SuperviseurPointeuses(fabrique, fabrique_surveillance)
        curseur.fetchall.return_value = [(1, "192.168.1.100"), (5, " 192.168.1.204 "), (6, "")]
        superviseur.actualiser()
        self.assertEqual(creees, [(1, "192.168.1.100"), (5, "192.168.1.204")])

        ancienne = superviseur._surveillances[1]
        curseur.fetchall.return_value = [(1, "192.168.1.101")]
        superviseur.actualiser()
        ancienne.arreter.assert_called_once()
        self.assertEqual(set(superviseur._surveillances), {1})
        self.assertEqual(creees[-1], (1, "192.168.1.101"))


if __name__ == "__main__":
    unittest.main()
