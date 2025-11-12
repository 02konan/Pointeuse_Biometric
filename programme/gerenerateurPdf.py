import os
from reportlab.lib.pagesizes import A4,A3
from reportlab.pdfgen import canvas
from reportlab.lib.units import cm
from datetime import datetime
from jinja2 import Template
from weasyprint import HTML
from programme.Creat_data import creat_rapports

def format_timedelta(tdelta):
    total_seconds = int(tdelta.total_seconds())
    sign = "-" if total_seconds < 0 else ""
    total_seconds = abs(total_seconds)

    hours = total_seconds // 3600
    minutes = (total_seconds % 3600) // 60
    seconds = total_seconds % 60

    return f"{sign}{hours:02d}:{minutes:02d}:{seconds:02d}"

def generer_fiche_presence_pdf(utilisateur, id_utilisateur, filename=None, data=None):
    try:
        if not data or len(data) == 0:
            print("⚠ Aucune donnée de présence fournie.")
            return False

        uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
        os.makedirs(uploads_dir, exist_ok=True)

        if filename is None:
            filename = 'fiche_emargement.pdf'
        file_path = os.path.join(uploads_dir, filename)

        creat_rapports(file_path, utilisateur, id_utilisateur, 'Presence')

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
                'ecart': format_timedelta(ecart),
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
                        <th>Classe</th>
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
                        <td>RTGL</td>
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

