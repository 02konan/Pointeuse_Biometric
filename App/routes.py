"""API REST consommée par l'application mobile Flutter.

Conventions :
  * toutes les routes sont préfixées par /api ;
  * l'authentification se fait par jeton porteur — en-tête
    « Authorization: Bearer <jeton> » — et non par cookie de session, afin
    que l'application web et le mobile puissent coexister ;
  * le jeton est signé (itsdangerous) avec la clé secrète de l'application
    et porte une durée de vie ; il n'y a donc rien à stocker côté serveur.

Le mobile n'accède jamais à MySQL : il passe exclusivement par ces routes.
"""

from functools import wraps

from flask import Blueprint, current_app, g, jsonify, request
from itsdangerous import BadSignature, SignatureExpired, URLSafeTimedSerializer
from werkzeug.security import check_password_hash, generate_password_hash

from programme import api_mobile
from programme.api_mobile import ErreurBaseDeDonnees, TableManquante

api = Blueprint("api_mobile", __name__, url_prefix="/api")

# Durée de validité du jeton (secondes) : 7 jours.
DUREE_JETON = 7 * 24 * 3600
SEL_JETON = "jeton-mobile"


# ---------------------------------------------------------------------------
# Jetons
# ---------------------------------------------------------------------------

def _serialiseur():
    return URLSafeTimedSerializer(current_app.secret_key, salt=SEL_JETON)


def creer_jeton(identite):
    return _serialiseur().dumps(identite)


def lire_jeton(jeton):
    """Retourne l'identité portée par le jeton, ou None s'il est invalide."""
    try:
        return _serialiseur().loads(jeton, max_age=DUREE_JETON)
    except (SignatureExpired, BadSignature):
        return None


def _jeton_de_la_requete():
    entete = request.headers.get("Authorization", "")
    if entete.startswith("Bearer "):
        return entete[7:].strip()
    return None


def authentification_requise(f):
    """Refuse la requête si le jeton est absent, invalide ou expiré."""

    @wraps(f)
    def decoree(*args, **kwargs):
        jeton = _jeton_de_la_requete()
        if not jeton:
            return jsonify({"success": False, "error": "Jeton manquant"}), 401
        identite = lire_jeton(jeton)
        if not identite:
            return jsonify({"success": False, "error": "Jeton invalide ou expiré"}), 401
        g.utilisateur = identite
        return f(*args, **kwargs)

    return decoree


# ---------------------------------------------------------------------------
# Mots de passe
# ---------------------------------------------------------------------------

def verifier_mot_de_passe(empreinte_stockee, mot_de_passe):
    """Accepte les mots de passe hachés et, en repli, ceux encore en clair.

    Les comptes existants sont stockés en clair ; ce repli permet la bascule
    progressive vers des empreintes sans invalider les comptes actuels.
    Toute modification via PUT /api/me/password enregistre une empreinte.
    """
    if empreinte_stockee is None:
        return False
    if str(empreinte_stockee).startswith(("pbkdf2:", "scrypt:", "argon2")):
        return check_password_hash(empreinte_stockee, mot_de_passe)
    return empreinte_stockee == mot_de_passe


# ---------------------------------------------------------------------------
# Gestion d'erreurs commune au blueprint
# ---------------------------------------------------------------------------

@api.errorhandler(TableManquante)
def _table_manquante(e):
    return jsonify({
        "success": False,
        "error": "Fonctionnalité indisponible : la migration "
                 "database/migrations/001_mobile.sql n'a pas été appliquée.",
    }), 503


@api.errorhandler(ErreurBaseDeDonnees)
def _erreur_base(e):
    current_app.logger.error("Erreur base de données (API mobile) : %s", e)
    return jsonify({"success": False, "error": "Base de données indisponible"}), 503


def _corps():
    return request.get_json(silent=True) or {}


# ---------------------------------------------------------------------------
# Authentification
# ---------------------------------------------------------------------------

@api.post("/login")
def login():
    """Connexion mobile.

    Deux formes acceptées :
      * {"username": "...", "password": "..."}  → compte d'administration
      * {"matricule": "..."}                    → enseignant
    """
    donnees = _corps()
    username = (donnees.get("username") or "").strip()
    password = donnees.get("password") or ""
    matricule = (donnees.get("matricule") or "").strip()

    if username:
        compte = api_mobile.compte_utilisateur(username)
        if not compte or not verifier_mot_de_passe(compte["mot_de_passe"], password):
            return jsonify({"success": False, "error": "Identifiants incorrects"}), 401
        identite = {
            "type": "utilisateur",
            "id": compte["id"],
            "nom": compte["nom"],
            "role": compte["role"],
            "section": compte["section"],
            "matricule": None,
        }
    elif matricule:
        compte = api_mobile.compte_employe(matricule)
        if not compte:
            return jsonify({"success": False, "error": "Matricule inconnu"}), 401
        identite = {
            "type": "employe",
            "id": None,
            "nom": matricule,
            "role": compte["role"],
            "section": compte["section"],
            "matricule": compte["matricule"],
        }
    else:
        return jsonify({
            "success": False,
            "error": "Fournir soit username + password, soit matricule",
        }), 400

    return jsonify({
        "success": True,
        "token": creer_jeton(identite),
        "expires_in": DUREE_JETON,
        "user": {k: v for k, v in identite.items() if k != "id"},
    })


@api.post("/logout")
@authentification_requise
def logout():
    """Le jeton étant sans état, la déconnexion se fait côté client.

    La route existe pour que l'application puisse confirmer la fin de
    session et purger son stockage sécurisé.
    """
    return jsonify({"success": True})


@api.post("/forgot-password")
def forgot_password():
    return jsonify({
        "success": False,
        "error": "Réinitialisation non disponible : aucun canal d'envoi "
                 "(email ou SMS) n'est configuré côté serveur. "
                 "Contactez l'administrateur.",
    }), 501


# ---------------------------------------------------------------------------
# Utilisateur
# ---------------------------------------------------------------------------

def _matricule_courant():
    """Matricule associé au jeton, ou None pour un compte d'administration."""
    return g.utilisateur.get("matricule")


@api.get("/me")
@authentification_requise
def me():
    matricule = _matricule_courant()
    if not matricule:
        # Compte d'administration : pas de fiche employé associée.
        return jsonify({"success": True, "user": g.utilisateur, "profil": None})
    return jsonify({
        "success": True,
        "user": g.utilisateur,
        "profil": api_mobile.profil(matricule),
    })


@api.put("/me")
@authentification_requise
def maj_me():
    matricule = _matricule_courant()
    if not matricule:
        return jsonify({"success": False, "error": "Aucune fiche employé associée"}), 403

    donnees = _corps()
    actuel = api_mobile.profil(matricule)
    if not actuel:
        return jsonify({"success": False, "error": "Profil introuvable"}), 404

    telephone = donnees.get("telephone", actuel["telephone"])
    email = donnees.get("email", actuel["email"])
    adresse = donnees.get("adresse", actuel["adresse"])

    if email and "@" not in str(email):
        return jsonify({"success": False, "error": "Adresse email invalide"}), 400

    api_mobile.maj_profil(matricule, telephone, email, adresse)
    return jsonify({"success": True, "profil": api_mobile.profil(matricule)})


@api.put("/me/password")
@authentification_requise
def maj_mot_de_passe():
    if g.utilisateur.get("type") != "utilisateur":
        return jsonify({
            "success": False,
            "error": "Les comptes enseignants n'ont pas de mot de passe",
        }), 403

    donnees = _corps()
    ancien = donnees.get("ancien_mot_de_passe") or ""
    nouveau = donnees.get("nouveau_mot_de_passe") or ""

    if len(nouveau) < 8:
        return jsonify({
            "success": False,
            "error": "Le nouveau mot de passe doit faire au moins 8 caractères",
        }), 400

    compte = api_mobile.compte_utilisateur(g.utilisateur["nom"])
    if not compte or not verifier_mot_de_passe(compte["mot_de_passe"], ancien):
        return jsonify({"success": False, "error": "Ancien mot de passe incorrect"}), 401

    api_mobile.enregistrer_mot_de_passe(compte["id"], generate_password_hash(nouveau))
    return jsonify({"success": True})


# ---------------------------------------------------------------------------
# Pointages
# ---------------------------------------------------------------------------

def _exige_matricule():
    matricule = _matricule_courant()
    if not matricule:
        return None, (jsonify({
            "success": False,
            "error": "Route réservée aux comptes enseignants",
        }), 403)
    return matricule, None


@api.get("/pointages")
@authentification_requise
def pointages():
    matricule, erreur = _exige_matricule()
    if erreur:
        return erreur
    return jsonify({
        "success": True,
        "pointages": api_mobile.historique(
            matricule,
            debut=request.args.get("debut"),
            fin=request.args.get("fin"),
            limite=request.args.get("limite", 60),
        ),
    })


@api.get("/pointages/today")
@authentification_requise
def pointages_du_jour():
    matricule, erreur = _exige_matricule()
    if erreur:
        return erreur
    return jsonify({"success": True, "pointage": api_mobile.journee_courante(matricule)})


@api.get("/pointages/stats")
@authentification_requise
def pointages_stats():
    matricule, erreur = _exige_matricule()
    if erreur:
        return erreur
    return jsonify({
        "success": True,
        "stats": api_mobile.statistiques(matricule, request.args.get("mois")),
    })


# ---------------------------------------------------------------------------
# Emploi du temps
# ---------------------------------------------------------------------------

@api.get("/programme")
@authentification_requise
def programme():
    matricule, erreur = _exige_matricule()
    if erreur:
        return erreur
    return jsonify({"success": True, "programme": api_mobile.programme(matricule)})


@api.get("/programme/today")
@authentification_requise
def programme_du_jour():
    matricule, erreur = _exige_matricule()
    if erreur:
        return erreur
    return jsonify({
        "success": True,
        "programme": api_mobile.programme_du_jour(matricule, request.args.get("jour")),
    })


@api.get("/programme/week")
@authentification_requise
def programme_semaine():
    matricule, erreur = _exige_matricule()
    if erreur:
        return erreur
    return jsonify({"success": True, "semaine": api_mobile.programme_semaine(matricule)})


# ---------------------------------------------------------------------------
# Classement
# ---------------------------------------------------------------------------

@api.get("/classement")
@authentification_requise
def classement():
    """Classement de la section de l'utilisateur pour un mois donné.

    La réponse porte aussi la position de l'appelant et ce qui lui manque
    pour gagner un rang : un objectif atteignable motive davantage qu'un
    numéro de place.
    """
    matricule, erreur = _exige_matricule()
    if erreur:
        return erreur

    section = g.utilisateur.get("section")
    donnees = api_mobile.classement(section, request.args.get("mois"))
    lignes = donnees["classement"]

    moi = next((l for l in lignes if l["matricule"] == matricule), None)
    if moi:
        moi = dict(moi)
        rang = moi["rang"]
        if rang > 1:
            dessus = lignes[rang - 2]
            manque = dessus["jours_complets"] - moi["jours_complets"]
            moi["pour_gagner_un_rang"] = {
                "rang_vise": rang - 1,
                "journees_manquantes": max(manque, 0),
                # À égalité de journées, ce sont les heures qui départagent.
                "heures_manquantes": round(
                    max(dessus["heures"] - moi["heures"], 0), 1)
                if manque <= 0 else 0,
            }

    donnees["moi"] = moi
    donnees["success"] = True
    return jsonify(donnees)


# ---------------------------------------------------------------------------
# Notifications
# ---------------------------------------------------------------------------

@api.get("/notifications")
@authentification_requise
def notifications():
    matricule, erreur = _exige_matricule()
    if erreur:
        return erreur
    return jsonify({
        "success": True,
        "notifications": api_mobile.notifications(matricule),
    })


@api.put("/notifications/<int:id_notification>/read")
@authentification_requise
def notification_lue(id_notification):
    matricule, erreur = _exige_matricule()
    if erreur:
        return erreur
    if not api_mobile.marquer_lue(matricule, id_notification):
        return jsonify({"success": False, "error": "Notification introuvable"}), 404
    return jsonify({"success": True})


@api.post("/me/device-token")
@authentification_requise
def enregistrer_appareil():
    """Enregistre le jeton Firebase du téléphone (notifications push)."""
    matricule, erreur = _exige_matricule()
    if erreur:
        return erreur
    donnees = _corps()
    jeton = (donnees.get("token") or "").strip()
    if not jeton:
        return jsonify({"success": False, "error": "Jeton Firebase manquant"}), 400
    api_mobile.enregistrer_appareil(matricule, jeton, donnees.get("plateforme", "inconnue"))
    return jsonify({"success": True})
