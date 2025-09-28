from flask import Flask, render_template, request,send_file, redirect, url_for, jsonify, send_from_directory, Response, session, flash
from programme.read_data import pointeuse,read_data_from_pr,verification_prof,pointage_invalid,read_raports,read_data_from_db,read_utilisateur,read_idsection,read_idrole,read_matricule, read_data_employe,read_data_presence,read_data_pointeuse,verification_utilisateur
from programme.Creat_data import creat_data_employee, creat_data_pointeuse,cret_User
from programme.detecteur import recuperation_emprientes,get_etats_pointeuses
from programme.attendance import listen_attendance,synchronisation_attendance,programme_valider
from programme.insertion import insertion_
from programme.transfert_empreintes import transfert_empreintes
from programme.enrollement import enroler_utilisateur
from werkzeug.utils import secure_filename
from programme.gerenerateurPdf import generer_fiche_presence_pdf,generer_presence_unique,generer_fiche_absence_pdf,generer_fiche_retards_pdf,generer_absence,generer_unique_presence,generer_presence,generer_retard
from flask_cors import CORS
from programme.base_donnee import connexion
from datetime import datetime,timedelta
import threading
from functools import wraps
from urllib.parse import unquote
from programme.eduflowApi import api_programme,sync_programme_periodique
import os
app = Flask(__name__, static_folder='static', template_folder='template')
app.secret_key = '&é1234azerty'
app.permanent_session_lifetime = timedelta(minutes=10)

CORS(app)
def init_session():
    """Initialise les variables de session si elles n'existent pas"""
    if 'user_type' not in session:
        session['user_type'] = None  # 'section' ou 'professeur'
    if 'section_id' not in session:
        session['section_id'] = None
    if 'professeur_code' not in session:
        session['professeur_code'] = None
    if 'connecter' not in session:
        session['connecter'] = False
@app.before_request
def before_request():
    init_session()

def login_required(f):
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
            session.clear()
            session.permanent = True
            session['connecter'] = True
            session['username'] = username
            session['identifiant'] = utilisateur['identifiants']
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
@app.route('/login_prof',methods=['GET','POST'])
def login_prof():
    if 'connecter' in session and session['connecter']:
        return redirect(url_for('index'))
    if request.method == 'POST':
        username1 = request.form['username1']
        code = request.form['code']
        utilisateur = verification_prof(code,username1)
        if utilisateur is None:
            flash("Identifiants incorrects. Veuillez réessayer.", "danger")
        elif utilisateur is not None and utilisateur:
            session.clear()
            session.permanent = True
            session['connecter'] = True
            session['username'] = username1
            session['role'] = utilisateur['nom_roles']
            session['section'] = utilisateur['Matricule']
            return redirect(url_for('index'))
        else:
            flash("Identifiants incorrects. Veuillez réessayer.", "danger")
    return render_template('login.html')        
@app.route('/')
def index():
    if 'connecter' not in session or not session['connecter']:
        return redirect(url_for('login'))

    pointeuses = get_etats_pointeuses()
    return render_template('index.html', active_page='index', pointeuses=pointeuses)

@app.route('/employee', methods=['POST'])
@role_required('admin')
def enregistrement():
    Nom=f"{request.form['nom']} {request.form['prenom']}"
    telephone = request.form['telephone']
    email = request.form['email']
    date = request.form['joinDate']
    poste = request.form['position']
    address = request.form['address']
    section = request.form['section']
    idEmploye = request.form['idEmploye']
    # photo = request.files.get('image')
    role_id=3
    
    # chemin = None
    # if photo and photo.filename != '':
    #     chemin = os.path.join('uploads', secure_filename(photo.filename))
    #     os.makedirs(os.path.dirname(chemin), exist_ok=True)
    #     photo.save(chemin)

    creat_data_employee(idEmploye, Nom, telephone, address, email, poste, date, section,role_id)
    flash("Employé enregistré avec succès !", "success")
    return redirect(url_for('intf_employee'))

@app.route('/api/dashboard', methods=['GET'])
@login_required
def dashboard_data():
    if session.get("role") == "professeur":
        # Dashboard professeur
        data_user = read_data_from_pr(session['section'])
        if data_user:
            total_Pointage, presents, retard, activites, total_absents = data_user
            return jsonify({
                'pointage': total_Pointage,
                'presents_user': presents,
                'retard_user': retard,
                'absents_user': total_absents,
                'activité_recentes_user': activites,
                'pourcentage_presents_user': round((presents / total_Pointage) * 100, 2) if total_Pointage else 0,
                'pourcentage_absents_user': round((total_absents / total_Pointage) * 100, 2) if total_Pointage else 0,
                'pourcentage_retards_user': round((retard / total_Pointage) * 100, 2) if total_Pointage else 0,
            })
    else:
        # Dashboard admin
        data = read_data_from_db(session['section'])
        if data:
            total_eleves, presents, retard, activites, total_absents, \
            employes_actifs_mois, jours_travailles_mois, employes_retard_mois = data
            
            return jsonify({
                'total_eleves': total_eleves,
                'presents': presents,
                'retard': retard,
                'absents': total_absents,
                'pourcentage_presents': round((presents / total_eleves) * 100, 2) if total_eleves else 0,
                'pourcentage_absents': round((total_absents / total_eleves) * 100, 2) if total_eleves else 0,
                'pourcentage_retards': round((retard / total_eleves) * 100, 2) if total_eleves else 0,
                'activité_recentes': activites,
                'employes_actifs_mois': employes_actifs_mois,
                'jours_travailles_mois': jours_travailles_mois,
                'employes_retard_mois': employes_retard_mois,
            })
    return jsonify({})

@app.route('/employee')
@role_required('admin')
def intf_employee():
    data = read_data_employe()
    id_employee=read_matricule()
    table = []
    table_info=[]
    for info_user in id_employee:
        lecture_info={
            'Code':info_user[0],
            'Nom':info_user[1]
        }
        table_info.append(lecture_info)
    for donnee in data:
        information = {
            'ID': donnee[0],
            'Matricule': donnee[1],
            'Nom': donnee[2],
            'Telephone': donnee[3],
            'Adresse': donnee[4],
            'email': donnee[5],
            'Poste': donnee[6],
            'section': donnee[8],
        }
        table.append(information)
    return render_template('employee.html', active_page='employee', resultats=table,user_id=table_info)

@app.route('/presence')
def intf_presence():
    data = read_data_presence(session['section'])
    table = []
    for donnee in data:
        code = donnee[0]
        Nom = donnee[1]
        arrive = donnee[2]
        depart = donnee[3]
        jour = donnee[4]
        date = donnee[5]
        Duree_initial = donnee[6]
        Statut = donnee[7]
        # heur_arrivee = donnee[8]
        temps_presence = donnee[9]
        if Statut == "Présent":
            couleur_st = "success"
        else:
            couleur_st = "danger"
        resultat = {
            'Matricule': code,
            'Nom': Nom,
            'arrivee': arrive,
            'depart': depart,
            'jour': jour,
            'Date': date,
            'Duree_initial': Duree_initial,
            'Duree_presence': temps_presence,
            'Statut': Statut,
            'couleur_statut': couleur_st
        }
        table.append(resultat)
    return render_template('presence.html', active_page='presence', resultats=table)

@app.route('/pointage_invalie')
@role_required('admin')
def intf_pointage_invalie():
    data = pointage_invalid(session['section'])
    table = []
    for donnee in data:
        code = donnee[0]
        Nom = donnee[1]
        jour_pointage = donnee[8]
        heure_pointage = donnee[4]
        crenau = f"{donnee[5]}-{donnee[6]}"
        Duree_cours = donnee[7]
        
        resultat = {
            'Matricule': code,
            'Nom': Nom,
            'heure_pointage': heure_pointage,
            'crenau': crenau,
            'jour_pointage': jour_pointage,
            'Duree_cours': Duree_cours
        }
        table.append(resultat)
    return render_template('pointage_invalide.html', active_page='pointage_invalie', resultats=table)
@app.route('/validation_pointage', methods=['GET'])
def validation_programme():
    data = pointage_invalid(session['section'])
    employe_id = request.args.get('id') or request.form.get('id')
    if not employe_id:
        flash("Aucun ID d'employé fourni.", "danger")
        return redirect(url_for('intf_pointage_invalie'))

    employe_id = employe_id

    for donnee in data:
        IDemploye = donnee[0]
        date_pointage = f"{donnee[2]} 17:30:00"
        idpointeuse = donnee[3]
        jour_pointage = donnee[8]
        Status="Départ enregistré"
        if employe_id == IDemploye:
            if programme_valider(IDemploye, date_pointage, idpointeuse, jour_pointage,Status):
                flash("Pointage invalide validé et ajouté avec succès !", "success")
            else:
                flash(date_pointage, "danger")
            break
    else:
        flash("Aucun pointage correspondant trouvé.", "danger")
    return redirect(url_for('intf_pointage_invalie'))
@app.route('/api/fiche_presence', methods=['POST'])

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

    pdfexecut = generer_fiche_presence_pdf(session['username'],session['identifiant'],chemin_pdf, data)
    if pdfexecut and os.path.exists(chemin_pdf):
        return jsonify({
            "type": "Présence",
            "nom": filename,
            "periode": f"{date_debut} → {date_fin}",
            "auteur": session['username'],
            "date": datetime.now().strftime("%Y-%m-%d %H:%M")
        })
    return jsonify({"success": False, "message": "Fichier PDF introuvable"}), 404
@app.route('/api/fiche_retards', methods=['POST'])

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

    pdfexecut = generer_fiche_retards_pdf(session['username'],session['identifiant'],chemin_pdf, data)

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

    pdfexecut = generer_fiche_absence_pdf(session['username'],session['identifiant'],chemin_pdf, data)

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
    uploads_list = sorted(os.listdir(uploads_dir), reverse=True)
    base_list = read_raports(session['username'])
    fichiers_base = {os.path.basename(row[0]) for row in base_list} if base_list else set()
    uploads_list = [f for f in uploads_list if f in fichiers_base]
    fichiers = []
    for nom in uploads_list:
        if nom.endswith(".pdf"):
            type_rapport = "Présence" if "presence" in nom.lower() else "Absence"
            fichiers.append({
                "nom": nom,
                "type": type_rapport,
                "auteur": session['username'],
                "date": datetime.fromtimestamp(
                    os.path.getctime(os.path.join(uploads_dir, nom))
                ).strftime("%d-%m-%Y %H:%M")
            })
    return jsonify(fichiers)
@app.route("/api/pointages/<matricule>", methods=["GET"])
def get_pointages(matricule):
    try:
        conn = connexion()
        cursor = conn.cursor()  
        query = """
            SELECT 
    DATE(p.date_pointage) AS date_pointage,
    MIN(TIME(p.date_pointage)) AS heure_entree,
    MAX(TIME(p.date_pointage)) AS heure_sortie,
    e.Nom AS Nom_Prenom
FROM 
    pointages p
JOIN 
    empreintes e ON e.IDEmploye = p.IDEmploye
WHERE 
    e.IDEmploye =%s
    AND DATE(p.date_pointage) = CURRENT_DATE()
GROUP BY 
    DATE(p.date_pointage), e.Nom
ORDER BY 
    date_pointage DESC;

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

@app.route('/api/programme/<matricule>', methods=['GET'])
def api_programme_route(matricule):
    try:
        conn = connexion()
        cursor = conn.cursor()
        query = """
            SELECT professeur_code,jour, type, heure_arrivee, heure_depart, duree_cours,professeur_nom, Matiere 
            FROM Programme
            WHERE professeur_code = %s
            ORDER BY jour DESC
        """
        cursor.execute(query, (matricule,))
        rows = cursor.fetchall()

        programmes = []
        for row in rows:
            programmes.append({
                "professeur_code": row[0],
                "journee": str(row[1]),
                "EmploiDuTmp": row[2],
                "heure_arrivee": str(row[3]),
                "heure_depart": str(row[4]),
                "duree_cours": str(row[5]),
                "professeur_nom": str(row[6]),
                "Matiere": str(row[7]),
            })

        return jsonify(programmes)

    except Exception as e:
        return jsonify({"error": str(e)}), 500
    
@app.route('/rapports')
def intf_rapports():
    return render_template('rapport.html', active_page='rapports')
@role_required('admin')

@app.route('/appareils')
@role_required('admin')
def intf_appareils():
    idsection= read_idsection()
    data= get_etats_pointeuses()
    return render_template('materiel.html', active_page='appareils',resultats=data,sections=idsection)

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

@app.route('/enrolement', methods=['POST'])
def Programme_Enrollement():
    if request.method == 'POST':
     idutilisateur = request.form.get('uid')
     codeutilisateur = request.form.get('user_id')
     utilisateur = request.form.get('name_user')
     idemprientes = request.form.get('finger_id')
    else:
     idutilisateur = request.args.get('uid')
     codeutilisateur = request.args.get('user_id')
     utilisateur = request.args.get('name_user')
     idemprientes = request.args.get('finger_id')
    succes=enroler_utilisateur(uid=idutilisateur,user_id=codeutilisateur, name=utilisateur, finger_index=idemprientes)
    if succes:
        flash(f"Utilisateur {utilisateur} enrôlé avec succès !", "success")
    else:
        erreur=flash("Erreur lors de l'enrôlement.", "danger")
        return render_template('employee.html', active_page='employee',erreur=erreur)


@app.route('/utilisateurs')
@role_required('admin')
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
    recuperation = threading.Thread(target=recuperation_emprientes)
    recuperation.daemon = True
    recuperation.start()
    thread_insertion = threading.Thread(target=insertion_)
    thread_insertion.daemon = True
    thread_insertion.start()
    # thread_transfert_empreintes = threading.Thread(target=transfert_empreintes)
    # thread_transfert_empreintes.daemon = True
    # thread_transfert_empreintes.start()
    # thread_synchronisation_attendance = threading.Thread(target=synchronisation_attendance)
    # thread_synchronisation_attendance.daemon = True
    # thread_synchronisation_attendance.start()
    # thread_sync_programme_periodique = threading.Thread(target=sync_programme_periodique,args=(180,))
    # thread_sync_programme_periodique.daemon = True
    # thread_sync_programme_periodique.start()
    # thread = threading.Thread(target=listen_attendance)
    # thread.daemon = True
    # thread.start()

    app.run(host='0.0.0.0',port=5000,debug=True)