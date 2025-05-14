from flask import Flask, render_template, request, redirect, url_for, jsonify, send_from_directory, Response
from flask_cors import CORS
from read_data import read_data_from_db, read_data_employe,read_data_presence,read_data_pointeuse
from database.db import db
from Creat_employee import creat_data_employee, creat_data_pointeuse
from datetime import datetime,timedelta
import threading
import os
from detecteur import recuperation_emprientes
from attendance import listen_attendance
from werkzeug.utils import secure_filename

app = Flask(__name__, static_folder='static', template_folder='template')

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
    return render_template('index.html', active_page='index')

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
    table = []
    for donnee in data:
        information = {
            'Matricule': donnee[1],
            'Nom': donnee[2],
            'Prenom': donnee[3],
            'Telephone': donnee[4],
            'image': donnee[8],
            'email': donnee[6],
            'section': donnee[10],
        }
        table.append(information)
    return render_template('employee.html', active_page='employee', resultats=table)

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
        heure_limite = datetime.strptime("9:15", "%H:%M")

        # Cas où l'heure d'arrivée est égale à l'heure de départ
        if arrivee == depart:
            if heure_arrivee > heure_limite:
                statut = "En retard"
                couleur = "warning"
            else:
                statut = "Présent"
                couleur = "success"
        else:
            if heure_arrivee > heure_limite:
                statut = "En retard"
                couleur = "warning"
            elif heur_travaille == "00:00:00":
                statut = "Absent"
                couleur = "danger"
            else:
                statut = "Présent"
                couleur = "success"

     if not depart:
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
@app.route('/rapports')
def intf_rapports():
    return render_template('rapport.html', active_page='rapports')

@app.route('/appareils')
def intf_appareils():
    data= read_data_pointeuse()
    table =[]
    for donnee in data:
        resultat={
            'ID': donnee[0],
            'Nom': donnee[1],
            'Modele': donnee[2],
            'Localisation': donnee[3],
            'AdresseIp': donnee[4],
            'Port': donnee[5],
            'Serie': donnee[6],
            'Type': donnee[7],
        }
        table.append(resultat)
    return render_template('materiel.html', active_page='appareils',resultats=table)

    
@app.route('/add-device', methods=['POST'])
def enregistrement_appareils():
    pointeuseN = request.form['pointeuseN']
    pointeuseM = request.form['pointeuseM']
    pointeuseP = request.form['pointeuseP']
    Adresseip = request.form['Adresseip']
    pointeusePort = request.form['pointeusePort']
    pointeuseSerie = request.form['pointeuseSerie']
    pointeuseType = request.form['pointeuseType']
    creat_data_pointeuse(pointeuseN, pointeuseM, pointeuseP, Adresseip, pointeusePort, pointeuseSerie, pointeuseType)
    return redirect(url_for('intf_appareils'))

@app.route('/parametres')
def intf_Parametres():
    return render_template('parametre.html', active_page='parametres')



if __name__ == '__main__':
    thread = threading.Thread(target=listen_attendance)
    thread.daemon = True
    thread.start()
    recuperation = threading.Thread(target=recuperation_emprientes)
    recuperation.daemon = True
    recuperation.start()

    app.run(debug=True)