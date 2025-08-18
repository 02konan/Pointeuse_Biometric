from flask import Flask, render_template, request,send_file, redirect, url_for, jsonify, send_from_directory, Response, session, flash
from programme.read_data import read_data_from_db,read_utilisateur,read_idsection,read_idrole,read_matricule, read_data_employe,read_data_presence,read_data_pointeuse,verification_utilisateur
from programme.Creat_data import creat_data_employee, creat_data_pointeuse,cret_User,creat_rapports
from programme.detecteur import recuperation_emprientes,get_etats_pointeuses
from programme.attendance import listen_attendance,programme_attendance
from werkzeug.utils import secure_filename
from programme.gerenerateurPdf import generer_fiche_presence_pdf,generer_presence_unique,generer_fiche_absence_pdf,generer_fiche_retards_pdf,generer_absence,generer_unique_presence,generer_presence,generer_retard
from flask_cors import CORS
from programme.base_donnee import connexion
from datetime import datetime,timedelta
import threading
from programme.eduflowApi import api_programme,sync_programme_periodique
import os
app = Flask(__name__, static_folder='static', template_folder='template')
app.secret_key = '&é1234azerty'
app.permanent_session_lifetime = timedelta(minutes=10)

CORS(app)
@app.before_request
def before_request():
    if 'connecter' not in session:
        session['connecter'] = False
def login_required(f):
    from functools import wraps
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if 'connecter' not in session or not session['connecter']:
            return redirect(url_for('login'))
        return f(*args, **kwargs)
    return decorated_function

def role_required(role):
    def decorator(f):
        from functools import wraps
        @wraps(f)
        def decorated_function(*args, **kwargs):
            if 'role' not in session or session['role'].lower() != role.lower():
                flash("Accès refusé : vous n'avez pas les droits nécessaires.", "danger")
                return redirect(url_for('index'))
            return f(*args, **kwargs)
        return decorated_function
    return decorator

@app.route('/login', methods=['GET','POST'])
def login():
    if 'connecter' in session and session['connecter']:
        return redirect(url_for('index'))
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        # Vérification des identifiants
        utilisateur = verification_utilisateur(username, password)
        if utilisateur:
            # Enregistrer l'utilisateur dans la session
            session.permanent = True
            session['connecter'] = True
            session['username'] = username
            session['role'] = utilisateur['nom_roles']
            session['section'] = utilisateur['id_section']
            # Redirection selon le rôle
            if utilisateur['nom_roles'].lower() == 'admin':
                return redirect(url_for('index'))
            elif utilisateur['nom_roles'].lower() == 'user':
                return redirect(url_for('index'))
            else:
                return redirect(url_for('index'))
        else:
            flash("Identifiants incorrects. Veuillez réessayer.", "danger")
    return render_template('login.html')

@app.route('/')
@login_required
def index():
    if 'connecter' not in session or not session['connecter']:
        return redirect(url_for('login'))

    pointeuses = get_etats_pointeuses()
    return render_template('index.html', active_page='index', pointeuses=pointeuses)

@app.route('/employee', methods=['POST'])
@login_required
@role_required('admin')
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
@login_required
def dashboard_data():
    data = read_data_from_db(session['section'])
    if data:
        total_eleves, presents, retard, activites, total_absents, \
        employes_actifs_mois, jours_travailles_mois, employes_retard_mois, \
        = data
        
        return jsonify({
            'total_eleves': total_eleves,
            'presents': presents,
            'retard': retard,
            'absents': total_absents,
            'pourcentage_presents': round((presents / total_eleves) * 100, 2),
            'pourcentage_absents': round((total_absents / total_eleves) * 100, 2),
            'pourcentage_retards': round((retard / total_eleves) * 100, 2),
            'activité_recentes': activites,
            
            # DONNÉES MENSUELLES (tout le mois)
            'employes_actifs_mois': employes_actifs_mois,
            'jours_travailles_mois': jours_travailles_mois,
            'employes_retard_mois': employes_retard_mois,
            
        })
    return jsonify({})

@app.route('/employee')
@login_required
@role_required('admin')
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
@login_required
def intf_presence():
    data = read_data_presence(session['section'])
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
        elif heure_arrivee > heure_limite and heur_travaille >= timedelta(hours=9):
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
@login_required
def api_fiche_presence():
    data_json = request.get_json()
    date_debut = data_json.get('date_debut')
    date_fin = data_json.get('date_fin')

    if not date_debut or not date_fin:
        return jsonify({'error': 'Les dates sont obligatoires.'}), 400

    data = generer_presence(date_debut, date_fin,session['section'])
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
        creat_rapports(filename, session['username'])
        return jsonify({
            "success": True,
            "type": "Présence",
            "nom": filename,
            "periode": f"{date_debut} → {date_fin}",
            "auteur": session['username'],
            "date": datetime.now().strftime("%Y-%m-%d %H:%M")
        })

@app.route('/api/fiche_retards', methods=['POST'])
@login_required
def fiche_retards():
    data_json = request.get_json()
    date_debut_retard = data_json.get('date_debut_retard')
    date_fin_retard = data_json.get('date_fin_retard')

    if not date_debut_retard or not date_fin_retard:
        return jsonify({'error': 'Les dates sont obligatoires.'}), 400

    data = generer_retard(date_debut_retard, date_fin_retard,session['section'])
    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    os.makedirs(uploads_dir, exist_ok=True)

    base_filename = f"ficheretards_{date_debut_retard}_au_{date_fin_retard}".replace(":", "-").replace("/", "-")
    filename = f"{base_filename}.pdf"
    chemin_pdf = os.path.join(uploads_dir, filename)

    compteur = 1
    while os.path.exists(chemin_pdf):
        filename = f"{base_filename}_{compteur}.pdf"
        chemin_pdf = os.path.join(uploads_dir, filename)
        compteur += 1

    pdfexecut = generer_fiche_retards_pdf(chemin_pdf, data)

    if pdfexecut and os.path.exists(chemin_pdf):
        return jsonify({
            "success": True,
            "type": "retards",
            "nom": filename,
            "periode": f"{date_debut_retard} → {date_fin_retard}",
            "auteur": "Système",
            "date": datetime.now().strftime("%Y-%m-%d %H:%M")
        })
    return jsonify({'error': 'Erreur lors de la génération du PDF'}), 500

@app.route('/api/fiche_absence', methods=['POST'])
@login_required
def fiche_absence():
    data_json = request.get_json()
    date_debut_absence = data_json.get('date_debut_absence')
    date_fin_absence = data_json.get('date_fin_absence')

    if not date_debut_absence or not date_fin_absence:
        return jsonify({'error': 'Les dates sont obligatoires.'}), 400

    data = generer_absence(date_debut_absence, date_fin_absence,session['section'])
    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    os.makedirs(uploads_dir, exist_ok=True)

    base_filename = f"ficheabsence_{date_debut_absence}_au_{date_fin_absence}".replace(":", "-").replace("/", "-")
    filename = f"{base_filename}.pdf"
    chemin_pdf = os.path.join(uploads_dir, filename)

    compteur = 1
    while os.path.exists(chemin_pdf):
        filename = f"{base_filename}_{compteur}.pdf"
        chemin_pdf = os.path.join(uploads_dir, filename)
        compteur += 1

    pdfexecut = generer_fiche_absence_pdf(chemin_pdf, data)

    if pdfexecut and os.path.exists(chemin_pdf):
        return jsonify({
            "success": True,
            "type": "absences",
            "nom": filename,
            "periode": f"{date_debut_absence} → {date_fin_absence}",
            "auteur": session['username'],
            "date": datetime.now().strftime("%Y-%m-%d %H:%M")
        })
    return jsonify({'error': 'Erreur lors de la génération du PDF'}), 500

@app.route('/api/fiche_presence_unique', methods=['POST'])
@login_required
def fiche_presence_unique():
    data_json = request.get_json()
    matricule = data_json.get('matricule')

    if not matricule:
        return jsonify({'error': 'Le matricule est obligatoire.'}), 400

    # Génération du nom de fichier
    data=generer_unique_presence(matricule)
    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    os.makedirs(uploads_dir, exist_ok=True)

    base_filename = f"fiche_presence_{matricule}_{datetime.now().strftime('%Y-%m-%d_%H-%M')}"
    filename = f"{base_filename}.pdf"
    chemin_pdf = os.path.join(uploads_dir, filename)

    compteur = 1
    while os.path.exists(chemin_pdf):
        filename = f"{base_filename}_{compteur}.pdf"
        chemin_pdf = os.path.join(uploads_dir, filename)
        compteur += 1

    # Appel de ta fonction de génération
    pdfexecut = generer_presence_unique(matricule, filename)

    if pdfexecut and os.path.exists(chemin_pdf):
        return jsonify({
            "success": True,
            "type": "presence_unique",
            "nom": filename,
            "auteur": "Système",
            "date": datetime.now().strftime("%Y-%m-%d %H:%M")
        })
    return jsonify({'error': 'Erreur lors de la génération du PDF'}), 500

@app.route('/telechargement/<nom>')
@login_required
def telecharger_rapport(nom):
    chemin = os.path.join('uploads', nom)
    if os.path.exists(chemin):
      return send_file(chemin, as_attachment=True)
    return "Fichier non trouvé", 404

@app.route('/impression/<nom>')
@login_required
def imprimer_rapport(nom):
    chemin = os.path.join('uploads', nom)
    if os.path.exists(chemin):
        return send_file(chemin)
    return "Fichier non trouvé", 404

@app.route('/suppression/<nom>', methods=['DELETE'])
@login_required
def supprimer_rapport(nom):
    chemin = os.path.join('uploads', nom)
    if os.path.exists(chemin):
        os.remove(chemin)
        return jsonify({"success": True})
    return jsonify({"error": "Fichier introuvable"}), 404

@app.route('/api/liste_rapports')
@login_required
@role_required('admin')
def liste_rapports():
    uploads_dir = os.path.join(os.path.dirname(__file__), 'uploads')
    fichiers = []

    for nom in sorted(os.listdir(uploads_dir), reverse=True):
        if nom.endswith(".pdf"):
            type_rapport = "Présence" if "presence" in nom.lower() else "Absence"
            fichiers.append({
                "nom": nom,
                "type": type_rapport,
                "auteur": session['username'],
                "date": datetime.fromtimestamp(os.path.getctime(os.path.join(uploads_dir, nom))).strftime("%Y-%m-%d %H:%M")
            })

    return jsonify(fichiers)

@app.route("/api/pointages/<matricule>", methods=["GET"])
def get_pointages(matricule):
    try:
        conn = connexion()
        cursor = conn.cursor()  
        query = """
            SELECT 
         DATE(date_pointage) AS date_pointage,
         MIN(TIME(date_pointage)) AS heure_entree,
         MAX(TIME(date_pointage)) AS heure_sortie,
         empreintes.Matricule as Nom_Prenom
         FROM 
         pointages, empreintes
         WHERE 
         empreintes.Matricule =%s and DATE(date_pointage) = CURRENT_DATE()
         ORDER BY 
         date_pointage DESC
        """
        cursor.execute(query, (matricule,))
        rows = cursor.fetchall()

        # Convertir manuellement en liste de dictionnaires
        result = []
        for row in rows:
            result.append({
                "date_pointage": str(row[0]),
                "heure_entree": str(row[1]),
                "heure_sortie": str(row[2]),
                "Nom_Prenom": str(row[3])
            })

        return jsonify(result)

    except Exception as e:
        return jsonify({"error": str(e)}), 500
    finally:
        cursor.close()
        conn.close()

@app.route('/rapports')
@login_required
def intf_rapports():
    return render_template('rapport.html', active_page='rapports')

@app.route('/appareils')
@login_required
@role_required('admin')
def intf_appareils():
    idsection= read_idsection()
    data= get_etats_pointeuses()
    return render_template('materiel.html', active_page='appareils',resultats=data,sections=idsection)

@app.route('/add-device', methods=['POST'])
@login_required
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
@login_required
def intf_Parametres():
    return render_template('parametre.html', active_page='parametres')

@app.route('/utilisateurs')
@login_required
def lister_utilisateurs():
    idrole=read_idrole()
    idsection= read_idsection()
    data=read_utilisateur()
    table = []
    if data is not None:
        for donnee in data:
            information = {
                'id': donnee[0],
                'Nom': donnee[1],
                'Email': donnee[2],
                'Motpass': donnee[3],
                'role': donnee[4],
                'section': donnee[5]
            }
            table.append(information)
    return render_template('utilisateurs.html',utilisateurs=table,roles=idrole, sections=idsection, active_page='utilisateurs')

@app.route('/utilisateurs/ajouter', methods=['GET', 'POST'])
@login_required
@role_required('admin')
def ajouter_utilisateur():
    if request.method == 'POST':
        NomUtilisateur = request.form['nomuser']
        EmailUtilisateur = request.form['Email']
        RoleUtilisateur = int(request.form['role'])
        SectionUtilisateur = int(request.form['section'])
        passUtilisateur = request.form['pass']
        cret_User(NomUtilisateur, EmailUtilisateur,passUtilisateur, RoleUtilisateur, SectionUtilisateur)
    return redirect(url_for('lister_utilisateurs'))

@app.route('/logout')
def logout():
    session.clear()
    flash("Déconnexion réussie", "success")
    return redirect(url_for('login'))

@app.route('/api/emploi_du_temps', methods=['GET'])
def api_eduflow():
    return api_programme()


if __name__ == '__main__':
    thread = threading.Thread(target=listen_attendance)
    thread.daemon = True
    thread.start()
    recuperation = threading.Thread(target=recuperation_emprientes)
    recuperation.daemon = True
    recuperation.start()
    thread_sync = threading.Thread(target=sync_programme_periodique, args=(300,))
    thread_sync.daemon = True
    thread_sync.start()
    thread_pointage = threading.Thread(target=programme_attendance)
    thread_pointage.daemon = True
    thread_pointage.start()

    app.run(host='0.0.0.0',port=5000,debug=False)