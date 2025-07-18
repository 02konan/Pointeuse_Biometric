import os
from reportlab.lib.pagesizes import A4
from reportlab.pdfgen import canvas
from reportlab.lib.units import cm
from read_data import generer_presence,generer_retard,generer_absence,generer_unique_presence

def format_timedelta(tdelta):
    total_seconds = int(tdelta.total_seconds())
    hours = total_seconds // 3600
    minutes = (total_seconds % 3600) // 60
    seconds = total_seconds % 60
    return f"{hours:02d}:{minutes:02d}:{seconds:02d}"
def generer_fiche_presence_pdf(filename=None, data=None):
    if data is None:
        data = generer_presence()

    # Définir le chemin du dossier uploads
    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    os.makedirs(uploads_dir, exist_ok=True)
    
    if filename is None:
        filename = 'fiche_presence.pdf'
    file_path = os.path.join(uploads_dir, filename)

    c = canvas.Canvas(file_path, pagesize=A4)
    width, height = A4

    # Titre du document
    c.setFont("Helvetica-Bold", 16)
    c.drawString(2 * cm, height - 2 * cm, "Fiche de Présence")

    # Fonction pour dessiner les en-têtes du tableau
    def dessiner_entete(y):
        c.setFont("Helvetica-Bold", 12)
        c.drawString(0.5 * cm, y, "ID")
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
            id_employe, matricule, date_pointage, heure_arrivee, heure_depart, temps_presence = row

            if y_position < 2 * cm:
                c.showPage()
                y_position = dessiner_entete(height - 2 * cm)

            c.setFont("Helvetica", 12)
            c.drawString(0.5 * cm, y_position, str(id_employe))
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
def generer_fiche_retards_pdf(filename=None, data=None):
    if data is None:
        data = generer_retard()

    # Définir le chemin du dossier uploads
    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    os.makedirs(uploads_dir, exist_ok=True)
    
    if filename is None:
        filename = 'fiche_presence.pdf'
    file_path = os.path.join(uploads_dir, filename)

    c = canvas.Canvas(file_path, pagesize=A4)
    width, height = A4

    # Titre du document
    c.setFont("Helvetica-Bold", 16)
    c.drawString(2 * cm, height - 2 * cm, "Fiche de retards")

    # Fonction pour dessiner les en-têtes du tableau
    def dessiner_entete(y):
        c.setFont("Helvetica-Bold", 12)
        c.drawString(0.5 * cm, y, "ID")
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
            id_employe, matricule, date_pointage, heure_arrivee, heure_depart, temps_presence = row

            if y_position < 2 * cm:
                c.showPage()
                y_position = dessiner_entete(height - 2 * cm)

            c.setFont("Helvetica", 12)
            c.drawString(0.5 * cm, y_position, str(id_employe))
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
def generer_fiche_absence_pdf(filename=None, data=None):
    if data is None:
        data = generer_absence()

    # Définir le chemin du dossier uploads
    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    os.makedirs(uploads_dir, exist_ok=True)
    
    if filename is None:
        filename = 'fiche_absence.pdf'
    file_path = os.path.join(uploads_dir, filename)

    c = canvas.Canvas(file_path, pagesize=A4)
    width, height = A4

    # Titre du document
    c.setFont("Helvetica-Bold", 16)
    c.drawString(2 * cm, height - 2 * cm, "Fiche d'absence")

    # Fonction pour dessiner les en-têtes du tableau
    def dessiner_entete(y):
        c.setFont("Helvetica-Bold", 12)
        c.drawString(0.5 * cm, y, "ID")
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
            id_employe, matricule, date_pointage, heure_arrivee, heure_depart, temps_presence = row

            if y_position < 2 * cm:
                c.showPage()
                y_position = dessiner_entete(height - 2 * cm)

            c.setFont("Helvetica", 12)
            c.drawString(0.5 * cm, y_position, str(id_employe))
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
def generer_presence_unique(Matricule=None, file_name=None, data=None):
    # Récupération des données de présence si non fournies
    if data is None:
        data = generer_unique_presence(Matricule)

    # Définir le chemin du dossier uploads
    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    os.makedirs(uploads_dir, exist_ok=True)

    # Nom du fichier PDF
    if file_name is None:
        file_name = "fiche_presence_unique.pdf"
    file_path = os.path.join(uploads_dir, file_name)

    # Création du PDF
    c = canvas.Canvas(file_path, pagesize=A4)
    width, height = A4

    # Titre du document
    c.setFont("Helvetica-Bold", 16)
    c.drawString(2 * cm, height - 2 * cm, "Fiche de présence individuelle")

    # Infos personnelles (exemple fictif, à adapter selon ta BDD)
    infos_perso = {
        "Nom": "KONE Adama",
        "Matricule": Matricule,
        "Poste": "Assistant RH",
        "Service": "Ressources Humaines"
    }

    # Affichage des infos personnelles
    y_info = height - 3 * cm
    c.setFont("Helvetica", 12)
    c.drawString(2 * cm, y_info, f"Nom : {infos_perso['Nom']}")
    y_info -= 0.5 * cm
    c.drawString(2 * cm, y_info, f"Matricule : {infos_perso['Matricule']}")
    y_info -= 0.5 * cm
    c.drawString(2 * cm, y_info, f"Poste : {infos_perso['Poste']}")
    y_info -= 0.5 * cm
    c.drawString(2 * cm, y_info, f"Service : {infos_perso['Service']}")

    # Espace avant tableau
    y_position = y_info - 1 * cm

    # En-têtes du tableau
    def dessiner_entete(y):
        c.setFont("Helvetica-Bold", 12)
        c.drawString(2 * cm, y, "Date")
        c.drawString(6 * cm, y, "Heure Arrivée")
        c.drawString(10 * cm, y, "Heure Départ")
        c.drawString(14 * cm, y, "Durée")
        c.line(1.5 * cm, y - 0.2 * cm, width - 1.5 * cm, y - 0.2 * cm)
        return y - 1 * cm

    y_position = dessiner_entete(y_position)

    # Remplissage du tableau
    for ligne in data:
        try:
            date_pointage, heure_arrivee, heure_depart, duree = ligne

            if y_position < 2 * cm:
                c.showPage()
                y_position = dessiner_entete(height - 2 * cm)

            c.setFont("Helvetica", 11)
            c.drawString(2 * cm, y_position, date_pointage.strftime("%Y-%m-%d"))
            c.drawString(6 * cm, y_position, str(heure_arrivee))
            c.drawString(10 * cm, y_position, str(heure_depart))
            c.drawString(14 * cm, y_position, str(duree))
            y_position -= 0.5 * cm

        except Exception as e:
            print("Erreur lors de l'ajout d'une ligne :", ligne)
            print("Exception :", e)
    c.save()
    return file_path

