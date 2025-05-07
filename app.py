from flask import Flask, render_template, request, redirect, url_for,jsonify
from flask_cors import CORS
from read_data import read_data_from_db
from database.db import db
from Creat_employee import creat_data_employee
import threading
import os
# import detecteur
from attendance import listen_attendance
from werkzeug.utils import secure_filename

app= Flask(__name__, static_folder='static', template_folder='template')
app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('LIEN_DE_LABASE')
app.config['SQLALCHEMY_BINDS'] = {
    'default': os.getenv('LIEN_DE_LABASE')
}
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db.init_app(app)
with app.app_context():
    db.create_all()
CORS(app)

@app.route('/')
def index():
    return render_template('index.html',active_page='index')

@app.route('/enregistrer_employer', methods=['POST'])
def enregistrement():
    nom = request.form['nom']
    prenom = request.form['prenom']
    telephone = request.form['telephone']
    email = request.form['email']
    date = request.form['joinDate']
    poste = request.form['position']
    address = request.form['address']
    photo = request.files.get('image')

    chemin = None
    if photo and photo.filename != '':
        chemin = f"uploads/{photo.filename}"
        photo.save(chemin)

    creat_data_employee(nom, prenom, telephone, address,email, poste,chemin,date)

    return render_template('employee.html', message="Employé enregistré avec succès", active_page='employee')

@app.route('/api/dashboard',methods=['GET'])
def dashboard_data():
    data = read_data_from_db()
    if data:
        total_eleves, presents, retard, activites,total_absents = data
        

        return jsonify({
            'total_eleves': total_eleves,
            'presents': presents,
            'retard': retard,
            'absents': total_absents,
            'pourcentage_presents': round((presents / total_eleves) * 100, 2),
            'pourcentage_absents': round((total_absents / total_eleves) * 100, 2),
            'pourcentage_retards': round((retard / total_eleves) * 100, 2),
            'activité_recentes':activites
        })
    return jsonify({})

@app.route('/employee')
def intf_employee():
    return render_template('employee.html',active_page='employee')

@app.route('/presence')
def intf_presence():
    return render_template('presence.html',active_page='presence')

@app.route('/rapports')
def intf_rapports():
    return render_template('rapport.html',active_page='rapports')

@app.route('/appareils')
def intf_appareils():
    return render_template('materiel.html',active_page='appareils')

@app.route('/Parametres')
def intf_Parametres():
    return render_template('Parametre.html',active_page='Parametres')

if __name__ =='__main__':
    thread = threading.Thread(target=listen_attendance)
    thread.daemon = True
    thread.start()
    app.run(debug=True)