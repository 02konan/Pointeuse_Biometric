import 'package:flutter/foundation.dart';

import '../models/modeles.dart';
import 'api.dart';

/// État d'authentification partagé par toute l'application.
class Session extends ChangeNotifier {
  Session(this.api);

  final ApiClient api;

  Utilisateur? utilisateur;
  Profil? profil;
  bool chargement = true;
  String? erreur;

  bool get connecte => utilisateur != null;

  /// Restaure la session au démarrage si un jeton valide est stocké.
  Future<void> demarrer() async {
    chargement = true;
    notifyListeners();
    try {
      if (await api.sessionOuverte()) {
        final donnees = await api.moi();
        utilisateur = donnees.utilisateur;
        profil = donnees.profil;
      }
    } on ErreurApi {
      // Jeton expiré ou serveur injoignable : on revient à l'écran de connexion.
      utilisateur = null;
      profil = null;
    } finally {
      chargement = false;
      notifyListeners();
    }
  }

  Future<bool> connexionEnseignant(String matricule) =>
      _connexion(() => api.connexionEnseignant(matricule));

  Future<bool> connexionAdministration(String nom, String motDePasse) =>
      _connexion(() => api.connexionAdministration(nom, motDePasse));

  Future<bool> _connexion(Future<Utilisateur> Function() action) async {
    chargement = true;
    erreur = null;
    notifyListeners();
    try {
      utilisateur = await action();
      // Le profil complet n'est disponible que pour les enseignants.
      if (utilisateur!.estEnseignant) {
        profil = (await api.moi()).profil;
      }
      return true;
    } on ErreurApi catch (e) {
      erreur = e.message;
      return false;
    } finally {
      chargement = false;
      notifyListeners();
    }
  }

  Future<void> deconnexion() async {
    await api.deconnexion();
    utilisateur = null;
    profil = null;
    erreur = null;
    notifyListeners();
  }

  void majProfil(Profil nouveau) {
    profil = nouveau;
    notifyListeners();
  }

  /// Ferme la session lorsqu'une requête signale un jeton expiré.
  void sessionExpiree() {
    utilisateur = null;
    profil = null;
    erreur = 'Session expirée, reconnectez-vous.';
    notifyListeners();
  }
}
