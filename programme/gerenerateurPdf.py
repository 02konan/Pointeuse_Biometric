import os
from reportlab.lib.pagesizes import A4,A3
from reportlab.pdfgen import canvas
from reportlab.lib.units import cm
from    datetime import datetime
from programme.Creat_data import creat_rapports
from programme.read_data import generer_retard,generer_absence,generer_unique_presence

def format_timedelta(tdelta):
    total_seconds = int(tdelta.total_seconds())
    hours = total_seconds // 3600
    minutes = (total_seconds % 3600) // 60
    seconds = total_seconds % 60
    return f"{hours:02d}:{minutes:02d}:{seconds:02d}"


def generer_fiche_presence_pdf(utilisateur, id_utilisateur, filename=None, data=None):
    try:
        # ✅ Vérification des données
        if data is None or len(data) == 0:
            print("⚠️ Aucune donnée de présence fournie.")
            return False
        
        uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
        os.makedirs(uploads_dir, exist_ok=True)

        # ✅ Fichier PDF à générer
        if filename is None:
            filename = 'fiche_presence.pdf'
        file_path = os.path.join(uploads_dir, filename)

        # ✅ Historiser la création dans ta base ou ton journal
        creat_rapports(file_path, utilisateur, id_utilisateur, 'Presence')

        # ✅ Initialisation du PDF
        c = canvas.Canvas(file_path, pagesize=A3)
        width, height = A3

        # ✅ Titre
        c.setFont("Helvetica-Bold", 16)
        c.drawString(2 * cm, height - 2 * cm, "Fiche de Présence")

        # ✅ Fonction pour entête
        def dessiner_entete(y):
            c.setFont("Helvetica-Bold", 12)
            # Positions centrées pour chaque colonne
            columns = [
                (4 * cm, "Nom & Prénom"),
                (7 * cm, "Jour"),
                (10 * cm, "Date"),
                (13 * cm, "Heure de Cours"),
                (17 * cm, "Heure Effectuées"),
                (21 * cm, "Écart d'heure"),
                (25 * cm, "Observation")
            ]
            for x, label in columns:
                c.drawCentredString(x, y, label)
            c.line(1 * cm, y - 0.2 * cm, width - 1 * cm, y - 0.2 * cm)
            return y - 1 * cm
        y_position = dessiner_entete(height - 3 * cm)

        # ✅ Boucle sur les lignes de données
        for row in data:
            try:
                professeur,jours, date, heure_cours, heure_effectuee, ecart, observation = row

                # Si on arrive en bas de page → nouvelle page + entête
                if y_position < 2 * cm:
                    c.showPage()
                    y_position = dessiner_entete(height - 2 * cm)

                c.setFont("Helvetica", 12)
                # Centrage des données sur chaque colonne
                values = [
                    (4 * cm, str(professeur)),
                    (7 * cm, str(jours)),
                    (10 * cm, str(date)),
                    (13 * cm, str(heure_cours)),
                    (17 * cm, str(heure_effectuee)),
                    (21 * cm, str(ecart)),
                    (25 * cm, str(observation))
                ]
                for x, val in values:
                    c.drawCentredString(x, y_position, val)
                y_position -= 0.5 * cm

            except Exception as e:
                print(f"❌ Erreur lors de l'ajout d'une ligne {row} :", e)

        c.save()

        # ✅ Vérification que le PDF a bien été créé
        if os.path.exists(file_path):
            print(f"✅ PDF généré : {file_path}")
            return True
        else:
            print("❌ Échec : fichier PDF non créé.")
            return False

    except Exception as e:
        print("❌ Erreur lors de la génération du PDF :", e)
        return False

def generer_fiche_retards_pdf(utilisateur,id_utilisateur,filename=None, data=None):
    if data is None:
        data = generer_retard()

    # Définir le chemin du dossier uploads
    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    os.makedirs(uploads_dir, exist_ok=True)
    
    if filename is None:
        filename = 'fiche_retard.pdf'
    file_path = os.path.join(uploads_dir, filename)
    if file_path:
        creat_rapports(file_path,utilisateur,id_utilisateur,'retard')

    c = canvas.Canvas(file_path, pagesize=A4)
    width, height = A4

    # Titre du document
    c.setFont("Helvetica-Bold", 16)
    c.drawString(2 * cm, height - 2 * cm, "Fiche de retards")

    # Fonction pour dessiner les en-têtes du tableau
    def dessiner_entete(y):
        c.setFont("Helvetica-Bold", 12)
        c.drawString(2.5 * cm, y, "Matricule")
        c.drawString(6 * cm, y, "Date de Pointage")
        c.drawString(10 * cm, y, "Heure d'Arrivée")
        c.drawString(14 * cm, y, "Heure de Départ")
        c.drawString(17.5 * cm, y, "Durée")
        c.line(0.5 * cm, y - 0.2 * cm, width - 0.5 * cm, y - 0.2 * cm)
        return y - 1 * cm

    y_position = dessiner_entete(height - 3 * cm)

    for row in data:
        try:
            matricule, date_pointage, heure_arrivee, heure_depart, temps_presence = row

            if y_position < 2 * cm:
                c.showPage()
                y_position = dessiner_entete(height - 2 * cm)

            c.setFont("Helvetica", 12)
            c.drawString(2.5 * cm, y_position, str(matricule))
            c.drawString(6 * cm, y_position, date_pointage.strftime("%Y-%m-%d"))
            c.drawString(10 * cm, y_position, format_timedelta(heure_arrivee))
            c.drawString(14 * cm, y_position, format_timedelta(heure_depart))
            c.drawString(17.5 * cm, y_position, format_timedelta(temps_presence))
            y_position -= 0.5 * cm

        except Exception as e:
            print("Erreur lors de l'ajout d'une ligne :", row)
            print("Exception :", e)

    c.save()

def generer_fiche_absence_pdf(utilisateur,id_utilisateur,filename=None, data=None):
    if data is None:
        data = generer_absence()

    # Définir le chemin du dossier uploads
    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    os.makedirs(uploads_dir, exist_ok=True)
    
    if filename is None:
        filename = 'fiche_absence.pdf'
    file_path = os.path.join(uploads_dir, filename)
    if file_path:
        creat_rapports(file_path,utilisateur,id_utilisateur,'Absence')

    c = canvas.Canvas(file_path, pagesize=A4)
    width, height = A4

    # Titre du document
    c.setFont("Helvetica-Bold", 16)
    c.drawString(2 * cm, height - 2 * cm, "Fiche d'absence")

    # Fonction pour dessiner les en-têtes du tableau
    def dessiner_entete(y):
        c.setFont("Helvetica-Bold", 12)
        c.drawString(2.5 * cm, y, "Matricule")
        c.drawString(6 * cm, y, "Date de Pointage")
        c.drawString(10 * cm, y, "Heure d'Arrivée")
        c.drawString(14 * cm, y, "Heure de Départ")
        c.drawString(17.5 * cm, y, "Durée")
        c.line(0.5 * cm, y - 0.2 * cm, width - 0.5 * cm, y - 0.2 * cm)
        return y - 1 * cm

    y_position = dessiner_entete(height - 3 * cm)

    for row in data:
        try:
            matricule, date_pointage, heure_arrivee, heure_depart, temps_presence = row

            if y_position < 2 * cm:
                c.showPage()
                y_position = dessiner_entete(height - 2 * cm)

            c.setFont("Helvetica", 12)
            c.drawString(2.5 * cm, y_position, str(matricule))
            c.drawString(6 * cm, y_position, date_pointage.strftime("%Y-%m-%d"))
            c.drawString(10 * cm, y_position, format_timedelta(heure_arrivee))
            c.drawString(14 * cm, y_position, format_timedelta(heure_depart))
            c.drawString(17.5 * cm, y_position, format_timedelta(temps_presence))
            y_position -= 0.5 * cm

        except Exception as e:
            print("Erreur lors de l'ajout d'une ligne :", row)
            print("Exception :", e)

    c.save()

def generer_presence_unique_pdf(utilisateur,id_utilisateur,filename=None, data=None):
    if data is None:
        data = generer_unique_presence()

    # Définir le chemin du dossier uploads
    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    os.makedirs(uploads_dir, exist_ok=True)

    # Nom du fichier PDF
    if filename is None:
        filename = "fiche_presence_unique.pdf"
    file_path = os.path.join(uploads_dir, filename)
    if file_path:
        creat_rapports(file_path,utilisateur,id_utilisateur,'Presence_unique')

    # Création du PDF
    c = canvas.Canvas(file_path, pagesize=A4)
    width, height = A4

    # Titre du document
    c.setFont("Helvetica-Bold", 16)
    c.drawString(2 * cm, height - 2 * cm, "Fiche de présence individuelle")

    
    # En-têtes du tableau
    def dessiner_entete(y):
        c.setFont("Helvetica-Bold", 12)
        c.drawString(2.5 * cm, y, "Nom&Prenom")
        c.drawString(6 * cm, y, "Heure de Cours")
        c.drawString(10 * cm, y, "Heure Effectuées")
        c.drawString(14 * cm, y, "Ecart d'heure")
        c.drawString(17.5 * cm, y, "Observation")
        c.line(0.5 * cm, y - 0.2 * cm, width - 0.5 * cm, y - 0.2 * cm)
        return y - 1 * cm

    y_position = dessiner_entete(height - 3 * cm)

    # Remplissage du tableau
    for row in data:
        try:
            Date, jour, heure_cours, heure_effectuer, Ecart,Observation= row

            if y_position < 2 * cm:
                c.showPage()
                y_position = dessiner_entete(height - 2 * cm)

            c.setFont("Helvetica", 12)
            c.drawString(2.5 * cm, y_position, str(Date))
            c.drawString(6 * cm, y_position, str(jour))
            c.drawString(10 * cm, y_position, str(heure_cours))
            c.drawString(14 * cm, y_position, str(heure_effectuer))
            c.drawString(17.5 * cm, y_position, str(Ecart))
            c.drawString(17.5 * cm, y_position, str(Observation))
            y_position -= 0.5 * cm

        except Exception as e:
            print("Erreur lors de l'ajout d'une ligne :", row)
            print("Exception :", e)
    c.save()
    return file_path

