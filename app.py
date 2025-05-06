from flask import Flask, render_template, request, redirect, url_for,jsonify
from flask_cors import CORS
from read_data import read_data_from_db
from database.db import db
from Creat_employee import creat_data_employee
import os
import detecteur
from attendance import listen_attendance

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
@app.route('/api/dashboard',methods=['GET'])
def dashboard_data():
    data = read_data_from_db()
    if data:
        total_eleves, presents, retard, activites = data
        absents = total_eleves - presents - retard

        return jsonify({
            'total_eleves': total_eleves,
            'presents': presents,
            'retard': retard,
            'absents': absents,
            'pourcentage_presents': round((presents / total_eleves) * 100, 2),
            'pourcentage_absents': round((absents / total_eleves) * 100, 2),
            'pourcentage_retards': round((retard / total_eleves) * 100, 2),
            'activité_recentes': activites
        })
    return jsonify({})
@app.route('/employee')
def intf_employee():
    return render_template('employee.html',active_page='employee')
@app.route('/presence')
def intf_presence():
    return render_template('presence.html',active_page='presence')
@app.route('/enregistrer_employer', methods=['POST'])
def enregistrement():
    nom = request.form['nom']
    prenom = request.form['prenom']
    telephone = request.form['telephone']
    email = request.form['email']
    creat_data_employee(nom, prenom, telephone, email)
    return render_template('employee.html',message="Employé enregistré avec succès",active_page='employee')
if __name__ =='__main__':
    app.run(debug=True)