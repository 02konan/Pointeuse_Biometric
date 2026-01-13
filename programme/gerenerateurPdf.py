import os
from reportlab.lib.pagesizes import A4,A3
from reportlab.pdfgen import canvas
from reportlab.lib.units import cm
from datetime import datetime, timedelta
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

        # --- Préparer et agréger les lignes + totaux ---
        def parse_to_timedelta(value):
            if isinstance(value, timedelta):
                return value
            if isinstance(value, (int, float)):
                return timedelta(hours=float(value))
            if isinstance(value, str):
                try:
                    parts = value.split(":")
                    if len(parts) == 3:
                        h, m, s = map(int, parts)
                        return timedelta(hours=h, minutes=m, seconds=s)
                    return timedelta(hours=float(value))
                except Exception:
                    return timedelta(0)
            return timedelta(0)

        lignes = []
        total_heures_effectuees = timedelta(0)
        total_heures_cours_td = timedelta(0)
        total_ecart = timedelta(0)

        for row in data:
            if not isinstance(row, (list, tuple)):
                print("⚠️ Ligne inattendue (non tuple) dans generer_fiche_presence_pdf :", row)
                continue
            if len(row) < 8:
                print("⚠️ Ligne incomplète dans les données de présence (attendu 8 champs) :", row)
                row = list(row) + [""] * (8 - len(row))

            professeur, jour_pointage, Section_name, date_pointage, total_heures_cours, total_heures_effectuer, ecart, observation = row

            he_effectue_td = parse_to_timedelta(total_heures_effectuer)
            he_cours_td = parse_to_timedelta(total_heures_cours)
            ecart_td = parse_to_timedelta(ecart)

            total_heures_effectuees += he_effectue_td
            total_heures_cours_td+= he_cours_td
            total_ecart += ecart_td

            lignes.append({
                'professeur': str(professeur),
                'jour': str(jour_pointage),
                'Section': str(Section_name),
                'date': str(date_pointage),
                'heures_cours': str(total_heures_cours),
                'heures_effectuees': format_timedelta(he_effectue_td),
                'ecart': format_timedelta(ecart_td),
                'observation': str(observation)
            })

        total_heures_effectuees_str = format_timedelta(total_heures_effectuees)
        total_heures_cours_str = format_timedelta(total_heures_cours_td)
        total_ecart_str = format_timedelta(total_ecart)

        # === Template HTML avec logo, tableau principal, totaux en-dessous, et signature ===
        template_html = """
        <!DOCTYPE html>
        <html lang="fr">
        <head>
            <meta charset="UTF-8">
            <title>Fiche de Présence</title>
            <style>
                body { font-family: DejaVu Sans, sans-serif; font-size:12px; margin:20px; }
                .header { overflow:auto; margin-bottom:10px; }
                .logo-box { width:150px; height:60px; border:1px solid #ccc; float:left; margin-right:10px; display:flex; align-items:center; justify-content:center; color:#666; font-size:12px;}
                h2 { text-align:center; margin:0; padding-top:10px; }
                table { width:100%; border-collapse:collapse; margin-top:10px }
                th, td { border:1px solid #333; padding:6px; text-align:center }
                .totals { width:40%; margin-top:10px; margin-left:auto; margin-right:0; }
                .totals-table { width:100%; border-collapse:collapse; }
                .totals-table th, .totals-table td { border:1px solid #333; padding:6px; text-align:right }
                .footer { clear:both; margin-top:40px; display:flex; justify-content:space-between; }
                .signature { width:30%; text-align:center; border-top:1px solid #333; padding-top:6px; }
            </style>
        </head>
        <body>
            <div class="header">
                <div class="logo-box">Logo</div>
                <h2>Fiche de Présence</h2>
            </div>

            <table>
                <thead>
                    <tr>
                        <th>Professeur</th>
                        <th>Jour</th>
                        <th>Date</th>
                        <th>Section</th>
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
                        <td>{{ l.Section }}</td>
                        <td>{{ l.date }}</td>
                        <td>{{ l.heures_cours }}</td>
                        <td>{{ l.heures_effectuees }}</td>
                        <td>{{ l.ecart }}</td>
                        <td>{{ l.observation }}</td>
                    </tr>
                {% endfor %}
                </tbody>
            </table>

            <div class="totals">
                <table class="totals-table">
                    <tr><th>Total Heures Cours</th><td>{{ total_heures_cours }}</td></tr>
                    <tr><th>Total Heures Effectuées</th><td>{{ total_heures_effectuees }}</td></tr>
                    <tr><th>Total Écart</th><td>{{ total_ecart }}</td></tr>
                </table>
            </div>

            <div class="footer">
                <div class="signature">Signature Professeur</div>
                <div class="signature">Signature Responsable</div>
                <div class="signature">Date</div>
            </div>
        </body>
        </html>
        """

        template = Template(template_html)
        html_content = template.render(lignes=lignes, total_heures_effectuees=total_heures_effectuees_str,total_heures_cours=total_heures_cours_str, total_ecart=total_ecart_str)

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
