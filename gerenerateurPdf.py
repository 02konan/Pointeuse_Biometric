import os
from reportlab.lib.pagesizes import A4
from reportlab.pdfgen import canvas
from reportlab.lib.units import cm
from read_data import generer_presence,generer_retard

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

    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    os.makedirs(uploads_dir, exist_ok=True)
    
    if filename is None:
        filename = 'fiche_retards.pdf'
    file_path = os.path.join(uploads_dir, filename)

    c = canvas.Canvas(file_path, pagesize=A4)
    width, height = A4

    c.setFont("Helvetica-Bold", 16)
    c.drawString(2 * cm, height - 2 * cm, "Fiche de Retards")

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

            if heure_arrivee > heure_depart:  # Vérifier si l'heure d'arrivée est après l'heure de départ
                continue

            c.setFont("Helvetica", 12)
            c.drawString(0.5 * cm, y_position, str(id_employe))
            c.drawString(2.5 * cm, y_position, str(matricule))
            c.drawString(6 * cm, y_position, date_pointage.strftime("%Y-%m-%d"))
            c.drawString(10 * cm, y_position, format_timedelta)
            c.drawString(14 * cm, y_position, format_timedelta(heure_depart))
            c.drawString(17.5 * cm, y_position, format_timedelta(temps_presence))
            y_position -= 0.5 * cm
        except Exception as e:
            print("Erreur lors de l'ajout d'une ligne :", row)
            print("Exception :", e)