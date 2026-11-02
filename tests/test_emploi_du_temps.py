import io
import unittest
from datetime import time
from unittest.mock import MagicMock

from programme.emploi_du_temps import EmploiDuTempsService, ErreurImport, MODELE_CSV


class LectureEtValidationTests(unittest.TestCase):
    def setUp(self):
        self.service = EmploiDuTempsService(connection_factory=MagicMock())

    def test_lit_le_modele_csv(self):
        lignes = self.service.lire_fichier("edt.csv", MODELE_CSV.encode("utf-8-sig"))
        cours = self.service.valider(lignes)

        self.assertEqual(len(cours), 2)
        self.assertEqual(cours[0]["professeur_code"], "PROF001")
        self.assertEqual(cours[0]["jour"], "Lundi")
        self.assertEqual(cours[0]["heure_arrivee"], "08:00:00")
        self.assertEqual(cours[0]["duree_cours"], "02:00:00")
        self.assertEqual(cours[1]["type_edt"], "modulaire")

    def test_entetes_et_valeurs_souples(self):
        contenu = (
            "Matricule,Nom,Jour,Matière,Début,Fin\n"
            "P2,Ada,mercredi,Maths,8h30,10h\n"
        ).encode("cp1252")
        lignes = self.service.lire_fichier("edt.csv", contenu)
        cours = self.service.valider(lignes, section_par_defaut="CDY")

        self.assertEqual(cours[0]["jour"], "Mercredi")
        self.assertEqual(cours[0]["heure_arrivee"], "08:30:00")
        self.assertEqual(cours[0]["duree_cours"], "01:30:00")
        self.assertEqual(cours[0]["section"], "CDY")
        self.assertEqual(cours[0]["type_edt"], "definitif")

    def test_lit_un_fichier_excel(self):
        from openpyxl import Workbook

        classeur = Workbook()
        feuille = classeur.active
        feuille.append(["matricule", "enseignant", "jour", "heure_debut", "heure_fin", "section"])
        feuille.append(["P3", "Bob", "Vendredi", time(14, 0), time(16, 45), "CDY"])
        tampon = io.BytesIO()
        classeur.save(tampon)

        cours = self.service.valider(self.service.lire_fichier("edt.xlsx", tampon.getvalue()))

        self.assertEqual(cours[0]["heure_depart"], "16:45:00")
        self.assertEqual(cours[0]["duree_cours"], "02:45:00")

    def test_refuse_tout_le_fichier_si_une_ligne_est_invalide(self):
        contenu = (
            "matricule;enseignant;jour;heure_debut;heure_fin;section\n"
            "P1;Ada;Lundi;08:00;10:00;CDY\n"
            "P1;Ada;Lundy;10:00;09:00;CDY\n"
            "P1;Ada;Lundi;08:00;11:00;CDY\n"
        ).encode()
        lignes = self.service.lire_fichier("edt.csv", contenu)

        with self.assertRaises(ErreurImport) as contexte:
            self.service.valider(lignes)

        erreurs = contexte.exception.erreurs
        self.assertEqual(len(erreurs), 2)
        self.assertIn("Ligne 3", erreurs[0])
        self.assertIn("jour", erreurs[0])
        self.assertIn("heure de fin", erreurs[0])
        self.assertIn("Ligne 4", erreurs[1])
        self.assertIn("ligne 2", erreurs[1])

    def test_signale_les_colonnes_manquantes(self):
        with self.assertRaises(ErreurImport) as contexte:
            self.service.lire_fichier("edt.csv", b"matricule;jour\nP1;Lundi\n")
        self.assertIn("enseignant", str(contexte.exception))

    def test_refuse_les_autres_formats(self):
        with self.assertRaises(ErreurImport):
            self.service.lire_fichier("edt.pdf", b"%PDF")


class ImportTests(unittest.TestCase):
    def _service(self, reponses):
        curseur = MagicMock()
        curseur.fetchone.side_effect = reponses
        conn = MagicMock()
        conn.cursor.return_value.__enter__.return_value = curseur
        fabrique = MagicMock()
        fabrique.return_value.__enter__.return_value = conn
        return EmploiDuTempsService(connection_factory=fabrique), curseur, conn

    def _cours(self):
        service = EmploiDuTempsService(connection_factory=MagicMock())
        return service.valider(service.lire_fichier("edt.csv", MODELE_CSV.encode()))

    def test_met_a_jour_un_cours_existant_et_ajoute_le_nouveau(self):
        # 1er cours : déjà présent (id 7) ; 2e : absent, l'enseignant a un id EduFlow 42.
        service, curseur, conn = self._service([("programme",), (7, 42), None, (42,)])

        bilan = service.importer(self._cours())

        self.assertEqual(bilan, {"ajoutes": 1, "mis_a_jour": 1})
        requetes = [appel.args[0] for appel in curseur.execute.call_args_list]
        self.assertTrue(any("UPDATE `programme`" in r for r in requetes))
        insertion = next(a for a in curseur.execute.call_args_list if "INSERT INTO" in a.args[0])
        self.assertEqual(insertion.args[1][0], 42)
        conn.commit.assert_called_once()
        self.assertIn("INSERT INTO `programme`", insertion.args[0])

    def test_lit_la_table_quelle_que_soit_sa_casse(self):
        service, curseur, _ = self._service([("programme",)])
        curseur.fetchall.return_value = [
            ("P1", "Ada", "Lundi", "8:00:00", "10:00:00", "2:00:00", "CDY", "definitif"),
        ]

        cours = service.lister()

        self.assertIn("FROM `programme`", curseur.execute.call_args.args[0])
        self.assertEqual(cours[0]["debut"], "08:00")
        self.assertEqual(cours[0]["duree"], "02:00")
        self.assertEqual(cours[0]["section"], "CDY")
        self.assertEqual(cours[0]["type"], "definitif")

    def test_signale_une_table_absente(self):
        service, _, _ = self._service([None])

        with self.assertRaises(ErreurImport) as contexte:
            service.lister()
        self.assertIn("introuvable", str(contexte.exception))


if __name__ == "__main__":
    unittest.main()
