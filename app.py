from flask import Flask, render_template, request,send_file, redirect, url_for, jsonify, send_from_directory, Response, session, flash
from programme.read_data import pointeuse,historique_pointage,read_matricule_section,read_data_from_pr,verification_prof,pointage_invalid,read_raports,read_data_from_db,read_utilisateur,read_idsection,read_idrole,read_matricule, read_data_employe,read_data_presence,read_data_pointeuse,verification_utilisateur,generer_absence,generer_unique_presence,generer_presence,generer_retard
from programme.Creat_data import creat_data_employee, creat_data_pointeuse,cret_User
from programme.detecteur import recuperation_emprientes,get_etats_pointeuses
from programme.attendance import listen_attendance,synchronisation_attendance,programme_valider
from programme.insertion import insertion_
from programme.read_superadmin import historique_data,read_data_Admin,read_pointage,read_admin_presence,pointage_admin_invalid,data_chatjs,generer_presence_admin
from programme.transfert_empreintes import transfert_empreintes
from programme.enrollement import enroler_utilisateur
from werkzeug.utils import secure_filename
from programme.gerenerateurPdf import generer_fiche_presence_pdf
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
    if 'user_type' not in session:
        session['user_type'] = None  
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
            if utilisateur['nom_roles'].lower() == 'superadmin':
                return redirect(url_for('index'))
            elif utilisateur['nom_roles'].lower() == 'admin':
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
        code = request.form['code']
        utilisateur = verification_prof(code)
        if utilisateur is None:
            flash("Identifiants incorrects. Veuillez réessayer.", "danger")
        elif utilisateur is not None and utilisateur:
            session.clear()
            session.permanent = True
            session['connecter'] = True
            session['username'] = code
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
def enregistrement():
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

    creat_data_employee(idEmploye, telephone, address, email, poste, date, section,role_id)
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

@app.route("/api/dashboard_admin", methods=["GET"])
def dashboard_admin():
   if session.get('role')=="superadmin":
        
        try:
            data_Admin = read_data_Admin()
            read_chatjs=data_chatjs()
            
            if data_Admin is None or read_chatjs is None:
                return jsonify({
                    "success": False,
                    "error": "Erreur lors de la récupération des données"
                }), 500

            # Déstructuration des données selon read_data_Admin()
            (
                total_employes,
                total_Presents,
                total_retard,
                activite_recentes,
                total_absents,
                employes_actifs_mois,
                jours_travailles_mois,
                employes_retard_mois
            ) = data_Admin
            
            (
                chartjs_Presents, 
                chartjs_retard, 
                chartjs_absents,
            )=read_chatjs
            labels=[]
            chartjs_Presents_formatees=[]
            chartjs_retard_formatees=[]
            chartjs_absents_formatees=[]

            for iteme_presents_chart in chartjs_Presents:
                labels.append(iteme_presents_chart[0])
                chartjs_Presents_formatees.append(iteme_presents_chart[1])

            for iteme_retard_chart in chartjs_retard:
                labels.append(iteme_retard_chart[0])
                chartjs_retard_formatees.append(iteme_retard_chart[1])

            for iteme_absents_chart in chartjs_absents:
                labels.append(iteme_absents_chart[0])
                chartjs_absents_formatees.append(iteme_absents_chart[1])


            # Formater les activités récentes
            activites_formatees = []
            for activite in activite_recentes:
                activites_formatees.append({
                    "nom": activite[0],  # Nom
                    "date_pointage": activite[1].strftime("%Y-%m-%d %H:%M:%S") if activite[1] else None,
                    "status": activite[2],  # Status
                    "section": activite[3]  # NomSection
                })

            # Réponse structurée
            return jsonify({
                "success": True,
                "total_employes": total_employes,
                "total_presents": total_Presents,
                "total_retard": total_retard,
                "total_absents": total_absents,
                "employes_actifs_mois": employes_actifs_mois,
                "jours_travailles_mois": jours_travailles_mois,
                "employes_retard_mois": employes_retard_mois,
                "activite_recentes": activites_formatees,
                "chartjs_Presents":chartjs_Presents_formatees,
                "chartjs_retard":chartjs_retard_formatees,
                "chartjs_absents":chartjs_absents_formatees,
                "labels":labels

            }), 200

        except ValueError as e:
            # Erreur de déstructuration des données
            return jsonify({
                "success": False,
                "error": "Format de données invalide",
                "details": str(e)
            }), 500
        
        except Exception as e:
            # Erreur inattendue
            return jsonify({
                "success": False,
                "error": "Erreur serveur interne",
                "details": str(e)
            }), 500

@app.route("/historique-activites")
def intf_historique():
    section = read_idsection()
    info_section = section
    table_info = []
    for NomSection in info_section:
        table_info.append({
            "Nom": NomSection[1]
        })

    # Récupérer les activités récentes selon le rôle
    activites_formatees = []
    if session['role'] == "superadmin":
        activite_recentes = historique_data() or []
    else:
        activite_recentes = historique_pointage(session['section']) or []
    # Formater les activités pour la vue
    for activite in activite_recentes:
        activites_formatees.append({
            "nom": activite[0],
            "date_pointage": activite[1].strftime("%d-%m-%Y") if activite[1] else None,
            "heure_pointage": activite[2],
            "status": activite[3],
            "section": activite[4]
        })

    return render_template('historique.html', active_page='pointage', historique=activites_formatees, sections=table_info)

@app.route('/employee')
@login_required
def intf_employee():
    data = read_data_employe(session['section'])
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
            'Matricule': donnee[0],
            'Telephone': donnee[1],
            'Nom': donnee[2],
            'Adresse': donnee[3],
            'email': donnee[4],
            'Poste': donnee[5],
            'section': donnee[6],
        }
        table.append(information)
    return render_template('employee.html', active_page='employee', resultats=table,user_id=table_info)

@app.route('/presence')
@login_required
def intf_presence():
    table = []
    table_admin = []
    if session.get('role') == "admin":
        data = read_data_presence(session['section'])
        for donnee in data:
            code = donnee[0]
            Nom = donnee[1]
            arrive = donnee[2]
            depart = donnee[3]
            jour = donnee[4]
            date = donnee[5]
            Duree_initial = donnee[6]
            Statut = donnee[7]
            temps_presence = donnee[9]
            couleur_st = "success" if Statut == "Présent" else "danger"
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
    elif session.get('role') == "superadmin":
        data_admin = read_admin_presence()
        for donnee in data_admin:
            code_admin = donnee[0]
            Nom_admin = donnee[1]
            arrive_admin = donnee[2]
            depart_admin = donnee[3]
            jour_admin = donnee[4]
            date_admin = donnee[5]
            Duree_initial_admin = donnee[6]
            Statut_admin = donnee[7]
            temps_presence_admin = donnee[9]
            couleur_st_admin = "success" if Statut_admin == "Présent" else "danger"
            resultat = {
                'Matricule': code_admin,
                'Nom': Nom_admin,
                'arrivee': arrive_admin,
                'depart': depart_admin,
                'jour': jour_admin,
                'Date': date_admin,
                'Duree_initial': Duree_initial_admin,
                'Duree_presence': temps_presence_admin,
                'Statut': Statut_admin,
                'couleur_statut': couleur_st_admin
            }
            table_admin.append(resultat)
    return render_template('presence.html', active_page='presence', resultats=table, resultats_admin=table_admin)

@app.route('/pointage_invalie')
@login_required
def intf_pointage_invalie():
    table = []
    table_admin = []
    if session.get('role') == "admin":
        data = pointage_invalid(session['section'])
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
    elif session.get('role') == "superadmin":
        data_admin = pointage_admin_invalid()
        for donnee in data_admin:
            code_admin = donnee[0]
            Nom_admin = donnee[1]
            jour_pointage_admin = donnee[8]
            heure_pointage_admin = donnee[4]
            crenau_admin = f"{donnee[5]}-{donnee[6]}"
            Duree_cours_admin = donnee[7]
            resultat = {
                'Matricule': code_admin,
                'Nom': Nom_admin,
                'heure_pointage': heure_pointage_admin,
                'crenau': crenau_admin,
                'jour_pointage': jour_pointage_admin,
                'Duree_cours': Duree_cours_admin
            }
            table_admin.append(resultat)
    return render_template('pointage_invalide.html', active_page='pointage_invalie', resultats=table, resultats_admin=table_admin)

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
        date_pointage = f"{donnee[2]} {donnee[6]}"
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
    try:
        data_json = request.get_json()
        date_debut = data_json.get('date_debut')
        date_fin = data_json.get('date_fin')
        idemployee = data_json.get('idEmploye')

        if not date_debut or not date_fin:
            return jsonify({'success': False, 'error': 'Les dates sont obligatoires.'}), 400

        section = session['section']
        username = session['username']
        identifiant = session['identifiant']

        if not section or not username or not identifiant:
            return jsonify({'success': False, 'error': 'Session invalide. Veuillez vous reconnecter.'}), 401

        data = generer_presence(date_debut, date_fin, idemployee, section)

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

        pdfexecut = generer_fiche_presence_pdf(username, identifiant, chemin_pdf, data)

        if pdfexecut and os.path.exists(chemin_pdf):
            flash("PDF généré avec succès !", "success")
            return jsonify({
                "success": True,
                "type": "Présence",
                "nom": filename,
                "periode": f"{date_debut} → {date_fin}",
                "auteur": username,
                "date": datetime.now().strftime("%Y-%m-%d %H:%M")
            }), 200
        flash("Erreur lors de la génération du PDF", "danger")
        return jsonify({'success': False, 'error': 'Erreur lors de la génération du PDF'}), 500

    except Exception as e:
        print("❌ Erreur dans /api/fiche_presence :", str(e))
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/api/fiche_presence_admin', methods=['POST'])
def api_fiche_presence_admin():
    try:
        data_admin_json = request.get_json()
        date_debut = data_admin_json.get('date_admin_debut')
        date_fin = data_admin_json.get('date_admin_fin')
        idemployee = data_admin_json.get('employeeid_admin')
        sectionid=data_admin_json.get('section_admin')

        if not date_debut or not date_fin:
            return jsonify({'success': False, 'error': 'Les dates sont obligatoires.'}), 400

        username = session['username']
        identifiant = session['identifiant']

        if not username or not identifiant:
            return jsonify({'success': False, 'error': 'Session invalide. Veuillez vous reconnecter.'}), 401

        try:
            sectionid = int(sectionid) if sectionid is not None and str(sectionid).strip() != '' else None
        except (ValueError, TypeError):
            sectionid = None

        if sectionid == 0:
            data = generer_presence_admin(date_debut, date_fin, idemployee)
        else:
            section_to_use = sectionid if sectionid is not None else session.get('section')
            data = generer_presence(date_debut, date_fin, idemployee, section_to_use)
            
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

        pdfexecut = generer_fiche_presence_pdf(username, identifiant, chemin_pdf, data)

        if pdfexecut and os.path.exists(chemin_pdf):
            flash("PDF généré avec succès !", "success")
            return jsonify({
                "success": True,
                "type": "Présence",
                "nom": filename,
                "periode": f"{date_debut} → {date_fin}",
                "auteur": username,
                "date": datetime.now().strftime("%Y-%m-%d %H:%M")
            }), 200

        return jsonify({'success': False, 'error': 'Erreur lors de la génération du PDF'}), 500

    except Exception as e:
        print("❌ Erreur dans /api/fiche_presence_admin :", str(e))
        return jsonify({'success': False, 'error': str(e)}), 500

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

@app.route('/rapports')
@login_required
def intf_rapports():
     table=[]
     for userall in read_matricule():
         lectureid={
             'Code':userall[0],
             'Nom':userall[1]
         }
         table.append(lectureid) 
     return render_template('rapport.html', active_page='rapports',all_user=table)

@app.route('/appareils')
@login_required
def intf_appareils():
    idsection= read_idsection()
    data= read_data_pointeuse()
    table_pointeuse=[]
    for donne in data:
        info_pointeuse={
            'Nom':donne[0],
            'etat':donne[6],
            'Model':donne[1],
            'AdresseIP':donne[3],
            'Emplacement':donne[2],
            'Serie':donne[4],
            'Type':donne[5]
        }
        table_pointeuse.append(info_pointeuse)
    return render_template('materiel.html', active_page='appareils',resultats=table_pointeuse,sections=idsection)

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
@login_required
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
    # recuperation = threading.Thread(target=recuperation_emprientes)
    # recuperation.daemon = True
    # recuperation.start()
    # etats_pointeuse = threading.Thread(target=get_etats_pointeuses)
    # etats_pointeuse.daemon = True
    # etats_pointeuse.start()
    # thread_insertion = threading.Thread(target=insertion_)
    # thread_insertion.daemon = True
    # thread_insertion.start()
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