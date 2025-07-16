from flask import Flask, render_template, request,send_file, redirect, url_for, jsonify, send_from_directory, Response, session, flash
from flask_cors import CORS
from read_data import read_data_from_db,read_matricule, read_data_employe,read_data_presence,read_data_pointeuse,vefification_utilisateur
from database.db import db
from Creat_data import creat_data_employee, creat_data_pointeuse
from datetime import datetime,timedelta
import threading
import os
from detecteur import recuperation_emprientes,get_etats_pointeuses
from attendance import listen_attendance
from werkzeug.utils import secure_filename
from gerenerateurPdf import generer_fiche_presence_pdf,generer_presence,generer_retard

app = Flask(__name__, static_folder='static', template_folder='template')
app.secret_key = '&é1234azerty'

# app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('LIEN_DE_LABASE')
# app.config['SQLALCHEMY_BINDS'] = {
#     'default': os.getenv('LIEN_DE_LABASE')
# }
# app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
# db.init_app(app)
# with app.app_context():
# db.create_all()

CORS(app)


@app.route('/')
def index():
    pointeuses = get_etats_pointeuses()
    return render_template('index.html',active_page='index', pointeuses=pointeuses)

@app.route('/employee', methods=['POST'])
def enregistrement():
    nom = request.form['nom']
    prenom = request.form['prenom']
    telephone = request.form['telephone']
    email = request.form['email']
    date = request.form['joinDate']
    poste = request.form['position']
    address = request.form['address']
    section = request.form['section']
    idEmploye = request.form['idEmploye']
    photo = request.files.get('image')
    
    chemin = None
    if photo and photo.filename != '':
        chemin = os.path.join('uploads', secure_filename(photo.filename))
        os.makedirs(os.path.dirname(chemin), exist_ok=True)
        photo.save(chemin)

    creat_data_employee(idEmploye, nom, prenom, telephone, address, email, poste, chemin, date, section)
    flash("Employé enregistré avec succès !", "success")
    return redirect(url_for('intf_employee'))

@app.route('/api/dashboard', methods=['GET'])
def dashboard_data():
    data = read_data_from_db()
    if data:
        total_eleves, presents, retard, activites, total_absents = data

        return jsonify({
            'total_eleves': total_eleves,
            'presents': presents,
            'retard': retard,
            'absents': total_absents,
            'pourcentage_presents': round((presents / total_eleves) * 100, 2),
            'pourcentage_absents': round((total_absents / total_eleves) * 100, 2),
            'pourcentage_retards': round((retard / total_eleves) * 100, 2),
            'activité_recentes': activites
        })
    return jsonify({})

@app.route('/employee')
def intf_employee():
    data = read_data_employe()
    id_employee=read_matricule()
    table = []
    for donnee in data:
        information = {
            'Matricule': donnee[1],
            'Nom': donnee[2],
            'Prenom': donnee[3],
            'Telephone': donnee[4],
            'image': donnee[8],
            'Adresse': donnee[5],
            'Poste': donnee[7],
            'email': donnee[6],
            'section': donnee[10],
        }
        table.append(information)
    return render_template('employee.html', active_page='employee', resultats=table,user_id=id_employee)

@app.route('/presence')
def intf_presence():
    data = read_data_presence()
    table = []
    for donnee in data:
     arrivee = donnee[3]
     depart = donnee[4]
     heur_travaille = donnee[5]
     statut = "Absent"
     couleur = "danger"

     if arrivee:
        if isinstance(arrivee, timedelta):
            total_seconds = int(arrivee.total_seconds())
            hours = total_seconds // 3600
            minutes = (total_seconds % 3600) // 60
            arrivee = f"{hours:02}:{minutes:02}"

        heure_arrivee = datetime.strptime(arrivee, "%H:%M")
        heure_limite = datetime.strptime("8:15", "%H:%M")

        if heure_arrivee <= heure_limite:
            statut = "Présent"
            couleur = "success"
        else:
            statut="En retard"
            couleur = "warning"
        if heur_travaille < timedelta(hours=9):
            statut = "Absent"
            couleur = "danger"

     resultat = {
        'ID_employe': donnee[0],
        'Nom': donnee[1],
        'Date': donnee[2],
        'arrivee': arrivee,
        'depart': depart,
        'Heures': heur_travaille,
        'Statut': statut,
        'couleur': couleur
     }

     table.append(resultat)
    
    return render_template('presence.html', active_page='presence', resultats=table)

@app.route('/api/fiche_presence', methods=['POST'])
def api_fiche_presence():
    data_json = request.get_json()
    date_debut = data_json.get('date_debut')
    date_fin = data_json.get('date_fin')

    if not date_debut or not date_fin:
        return jsonify({'error': 'Les dates sont obligatoires.'}), 400

    data = generer_presence(date_debut, date_fin)
    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    os.makedirs(uploads_dir, exist_ok=True)

    base_filename = f"fichePresence_{date_debut}_au_{date_fin}".replace(":", "-").replace("/", "-")
    filename = f"{base_filename}.pdf"
    chemin_pdf = os.path.join(uploads_dir, filename)

    compteur = 1
    while os.path.exists(chemin_pdf):
        filename = f"{base_filename}_{compteur}.pdf"
        chemin_pdf = os.path.join(uploads_dir, filename)
        compteur += 1

    pdfexecut = generer_fiche_presence_pdf(chemin_pdf, data)

    if pdfexecut and os.path.exists(chemin_pdf):
        send_file(chemin_pdf, as_attachment=True)
        return jsonify({
            "success": True,
            "type": "Présence",
            "nom": filename,
            "periode": f"{date_debut} → {date_fin}",
            "auteur": "Système",
            "date": datetime.now().strftime("%Y-%m-%d %H:%M")
        })

@app.route('/api/fiche_absence', methods=['POST'])
def fiche_absence():
    data_json = request.get_json()
    date_debut_retard = data_json.get('date_debut')
    date_fin_retard = data_json.get('date_fin')

    if not date_debut_retard or not date_fin_retard:
        return jsonify({'error': 'Les dates sont obligatoires.'}), 400

    data = generer_retard(date_debut_retard, date_fin_retard)
    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    os.makedirs(uploads_dir, exist_ok=True)

    base_filename = f"ficheAbsence_{date_debut_retard}_au_{date_fin_retard}".replace(":", "-").replace("/", "-")
    filename = f"{base_filename}.pdf"
    chemin_pdf = os.path.join(uploads_dir, filename)

    compteur = 1
    while os.path.exists(chemin_pdf):
        filename = f"{base_filename}_{compteur}.pdf"
        chemin_pdf = os.path.join(uploads_dir, filename)
        compteur += 1

    pdfexecut = generer_fiche_presence_pdf(chemin_pdf, data)

    if pdfexecut and os.path.exists(chemin_pdf):
        return jsonify({
            "success": True,
            "type": "Absence",
            "nom": filename,
            "periode": f"{date_debut_retard} → {date_fin_retard}",
            "auteur": "Système",
            "date": datetime.now().strftime("%Y-%m-%d %H:%M")
        })

    return jsonify({'error': 'Erreur lors de la génération du PDF'}), 500

@app.route('/telechargement/<nom>')
def telecharger_rapport(nom):
    chemin = os.path.join('uploads', nom)
    if os.path.exists(chemin):
      return send_file(chemin, as_attachment=True)
    return "Fichier non trouvé", 404

@app.route('/impression/<nom>')
def imprimer_rapport(nom):
    chemin = os.path.join('uploads', nom)
    if os.path.exists(chemin):
        return send_file(chemin)
    return "Fichier non trouvé", 404

@app.route('/suppression/<nom>', methods=['DELETE'])
def supprimer_rapport(nom):
    chemin = os.path.join('uploads', nom)
    if os.path.exists(chemin):
        os.remove(chemin)
        return jsonify({"success": True})
    return jsonify({"error": "Fichier introuvable"}), 404
@app.route('/api/liste_rapports')
def liste_rapports():
    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    fichiers = []

    for nom in sorted(os.listdir(uploads_dir), reverse=True):
        if nom.endswith(".pdf"):
            type_rapport = "Présence" if "presence" in nom.lower() else "Absence"
            fichiers.append({
                "nom": nom,
                "type": type_rapport,
                "periode": "Inconnue",  # Tu peux parser depuis le nom si besoin
                "auteur": "Système",
                "date": datetime.fromtimestamp(os.path.getctime(os.path.join(uploads_dir, nom))).strftime("%Y-%m-%d %H:%M")
            })

    return jsonify(fichiers)

@app.route('/rapports')
def intf_rapports():
    return render_template('rapport.html', active_page='rapports')

@app.route('/appareils')
def intf_appareils():
    data= get_etats_pointeuses()
    
    return render_template('materiel.html', active_page='appareils',resultats=data)
    
@app.route('/add-device', methods=['POST'])
def enregistrement_appareils():
    pointeuseN = request.form['pointeuseN']
    pointeuseM = request.form['pointeuseM']
    pointeuseP = request.form['pointeuseP']
    Adresseip = request.form['Adresseip']
    pointeuseSerie = request.form['pointeuseSerie']
    pointeuseType = request.form['pointeuseType']
    creat_data_pointeuse(pointeuseN, pointeuseM, pointeuseP, Adresseip,pointeuseSerie, pointeuseType)
    flash("Appareil enregistré avec succès !", "success")
    return redirect(url_for('intf_appareils'))

@app.route('/parametres')
def intf_Parametres():
    return render_template('parametre.html', active_page='parametres')

@app.route('/logout')
def logout():
    session.pop('connecter', None)
    return redirect(url_for('login'))


if __name__ == '__main__':
    thread = threading.Thread(target=listen_attendance)
    thread.daemon = True
    thread.start()
    recuperation = threading.Thread(target=recuperation_emprientes)
    recuperation.daemon = True
    recuperation.start()

    app.run(debug=True)