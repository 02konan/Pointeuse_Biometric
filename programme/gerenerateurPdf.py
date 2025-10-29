import os
from reportlab.lib.pagesizes import A4,A3
from reportlab.pdfgen import canvas
from reportlab.lib.units import cm
from datetime import datetime
from jinja2 import Template
from weasyprint import HTML
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
        if not data or len(data) == 0:
            print("⚠ Aucune donnée de présence fournie.")
            return False

        uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
        os.makedirs(uploads_dir, exist_ok=True)

        # ✅ Nom du fichier PDF
        if filename is None:
            filename = 'fiche_emargement.pdf'
        file_path = os.path.join(uploads_dir, filename)

        # ✅ Historiser la création du rapport
        creat_rapports(file_path, utilisateur, id_utilisateur, 'Presence')

        # Préparer les lignes pour le template HTML
        lignes = []
        for row in data:
            if not isinstance(row, (list, tuple)):
                print("⚠️ Ligne inattendue (non tuple) dans generer_fiche_presence_pdf :", row)
                continue
            if len(row) < 7:
                print("⚠️ Ligne incomplète dans les données de présence (attendu 7 champs) :", row)
                # compléter avec des valeurs vides
                row = list(row) + [""] * (7 - len(row))
            professeur, jour_pointage, date_pointage, total_heures_cours, total_heures_effectuer, ecart, observation = row
            lignes.append({
                'professeur': str(professeur),
                'jour': str(jour_pointage),
                'date': str(date_pointage),
                'heures_cours': str(total_heures_cours),
                'heures_effectuees': str(total_heures_effectuer),
                'ecart': str(ecart),
                'observation': str(observation)
            })

        # === Template HTML simplifié et adapté aux données ===
        template_html = """
        <!DOCTYPE html>
        <html lang="fr">
        <head>
            <meta charset="UTF-8">
            <title>Fiche de Présence</title>
            <style>
                body { font-family: DejaVu Sans, sans-serif; font-size:12px; margin:20px; }
                h2 { text-align:center; }
                table { width:100%; border-collapse:collapse; margin-top:10px }
                th, td { border:1px solid #333; padding:6px; text-align:center }
            </style>
        </head>
        <body>
            <h2>Fiche de Présence</h2>
            <table>
                <thead>
                    <tr>
                        <th>Professeur</th>
                        <th>Jour</th>
                        <th>Date</th>
                        <th>Heures de Cours</th>
                        <th>Heures Effectuées</th>
                        <th>Écart</th>
                        <th>Observation</th>
                    </tr>
                </thead>
                <tbody>
                {% for l in lignes %}
                    <tr>
                        <td>{{ l.professeur }}</td>
                        <td>{{ l.jour }}</td>
                        <td>{{ l.date }}</td>
                        <td>{{ l.heures_cours }}</td>
                        <td>{{ l.heures_effectuees }}</td>
                        <td>{{ l.ecart }}</td>
                        <td>{{ l.observation }}</td>
                    </tr>
                {% endfor %}
                </tbody>
            </table>
        </body>
        </html>
        """

        template = Template(template_html)
        html_content = template.render(lignes=lignes)

        # Génération du PDF avec WeasyPrint
        HTML(string=html_content).write_pdf(file_path)

        if os.path.exists(file_path):
            print(f"✅ Fiche d’émargement générée : {file_path}")
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

