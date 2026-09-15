import 'dart:convert';

import 'package:flutter_secure_storage/flutter_secure_storage.dart';
import 'package:http/http.dart' as http;

import '../config.dart';
import '../models/modeles.dart';

/// Erreur métier remontée par l'API, porteuse d'un message affichable.
class ErreurApi implements Exception {
  final String message;
  final int? code;
  const ErreurApi(this.message, [this.code]);

  /// Vrai lorsque la session doit être fermée (jeton absent, invalide, expiré).
  bool get sessionExpiree => code == 401;

  @override
  String toString() => message;
}

/// Client HTTP de l'API Flask.
///
/// Le jeton n'est jamais écrit en clair : il vit dans le stockage sécurisé
/// du système (Keychain sur iOS, EncryptedSharedPreferences sur Android),
/// conformément au point 13 du plan. Aucun mot de passe n'est conservé.
class ApiClient {
  ApiClient({http.Client? client}) : _http = client ?? http.Client();

  final http.Client _http;
  static const _stockage = FlutterSecureStorage(
    aOptions: AndroidOptions(encryptedSharedPreferences: true),
  );
  static const _cleJeton = 'jeton_api';

  String? _jetonMemoire;

  // -------------------------------------------------------------------------
  // Jeton
  // -------------------------------------------------------------------------

  Future<String?> jeton() async {
    _jetonMemoire ??= await _stockage.read(key: _cleJeton);
    return _jetonMemoire;
  }

  Future<void> _enregistrerJeton(String valeur) async {
    _jetonMemoire = valeur;
    await _stockage.write(key: _cleJeton, value: valeur);
  }

  Future<void> effacerJeton() async {
    _jetonMemoire = null;
    await _stockage.delete(key: _cleJeton);
  }

  Future<bool> sessionOuverte() async => (await jeton()) != null;

  // -------------------------------------------------------------------------
  // Transport
  // -------------------------------------------------------------------------

  Uri _uri(String chemin, [Map<String, String>? parametres]) =>
      Uri.parse('${Config.apiUrl}$chemin').replace(queryParameters: parametres);

  Future<Map<String, String>> _entetes({bool avecCorps = false}) async {
    final entetes = <String, String>{'Accept': 'application/json'};
    if (avecCorps) entetes['Content-Type'] = 'application/json';
    final t = await jeton();
    if (t != null) entetes['Authorization'] = 'Bearer $t';
    return entetes;
  }

  Future<Map<String, dynamic>> _traiter(http.Response reponse) async {
    Map<String, dynamic> corps;
    try {
      corps = jsonDecode(reponse.body) as Map<String, dynamic>;
    } catch (_) {
      throw ErreurApi('Réponse illisible du serveur', reponse.statusCode);
    }

    if (reponse.statusCode >= 200 && reponse.statusCode < 300) return corps;

    // Un jeton expiré invalide la session : on purge le stockage tout de suite.
    if (reponse.statusCode == 401) await effacerJeton();

    throw ErreurApi(
      corps['error']?.toString() ?? 'Erreur ${reponse.statusCode}',
      reponse.statusCode,
    );
  }

  Future<Map<String, dynamic>> _get(String chemin,
      [Map<String, String>? parametres]) async {
    try {
      final r = await _http
          .get(_uri(chemin, parametres), headers: await _entetes())
          .timeout(const Duration(seconds: 20));
      return _traiter(r);
    } on ErreurApi {
      rethrow;
    } catch (_) {
      throw const ErreurApi('Serveur injoignable. Vérifiez votre connexion.');
    }
  }

  Future<Map<String, dynamic>> _envoyer(
    String methode,
    String chemin, [
    Map<String, dynamic>? corps,
  ]) async {
    try {
      final entetes = await _entetes(avecCorps: true);
      final uri = _uri(chemin);
      final donnees = jsonEncode(corps ?? {});
      final r = await (methode == 'PUT'
              ? _http.put(uri, headers: entetes, body: donnees)
              : _http.post(uri, headers: entetes, body: donnees))
          .timeout(const Duration(seconds: 20));
      return _traiter(r);
    } on ErreurApi {
      rethrow;
    } catch (_) {
      throw const ErreurApi('Serveur injoignable. Vérifiez votre connexion.');
    }
  }

  // -------------------------------------------------------------------------
  // Authentification
  // -------------------------------------------------------------------------

  /// Connexion enseignant, par matricule.
  Future<Utilisateur> connexionEnseignant(String matricule) =>
      _connexion({'matricule': matricule});

  /// Connexion d'un compte d'administration.
  Future<Utilisateur> connexionAdministration(String nom, String motDePasse) =>
      _connexion({'username': nom, 'password': motDePasse});

  Future<Utilisateur> _connexion(Map<String, dynamic> identifiants) async {
    final corps = await _envoyer('POST', '/api/login', identifiants);
    final t = corps['token']?.toString();
    if (t == null) throw const ErreurApi('Jeton absent de la réponse');
    await _enregistrerJeton(t);
    return Utilisateur.depuisJson(
        (corps['user'] as Map?)?.cast<String, dynamic>() ?? {});
  }

  Future<void> deconnexion() async {
    // La déconnexion locale prime : même si l'appel échoue, le jeton part.
    try {
      await _envoyer('POST', '/api/logout');
    } catch (_) {
      // sans conséquence : le jeton est sans état côté serveur
    }
    await effacerJeton();
  }

  // -------------------------------------------------------------------------
  // Profil
  // -------------------------------------------------------------------------

  Future<({Utilisateur utilisateur, Profil? profil})> moi() async {
    final corps = await _get('/api/me');
    final profilJson = (corps['profil'] as Map?)?.cast<String, dynamic>();
    return (
      utilisateur: Utilisateur.depuisJson(
          (corps['user'] as Map?)?.cast<String, dynamic>() ?? {}),
      profil: profilJson == null ? null : Profil.depuisJson(profilJson),
    );
  }

  Future<Profil> majProfil({String? telephone, String? email, String? adresse}) async {
    final corps = await _envoyer('PUT', '/api/me', {
      if (telephone != null) 'telephone': telephone,
      if (email != null) 'email': email,
      if (adresse != null) 'adresse': adresse,
    });
    return Profil.depuisJson(
        (corps['profil'] as Map?)?.cast<String, dynamic>() ?? {});
  }

  Future<void> changerMotDePasse(String ancien, String nouveau) =>
      _envoyer('PUT', '/api/me/password', {
        'ancien_mot_de_passe': ancien,
        'nouveau_mot_de_passe': nouveau,
      });

  // -------------------------------------------------------------------------
  // Pointages
  // -------------------------------------------------------------------------

  Future<List<Pointage>> pointages({String? debut, String? fin, int limite = 60}) async {
    final corps = await _get('/api/pointages', {
      if (debut != null) 'debut': debut,
      if (fin != null) 'fin': fin,
      'limite': '$limite',
    });
    return ((corps['pointages'] as List?) ?? [])
        .map((e) => Pointage.depuisJson((e as Map).cast<String, dynamic>()))
        .toList();
  }

  Future<Pointage> pointageDuJour() async {
    final corps = await _get('/api/pointages/today');
    return Pointage.depuisJson(
        (corps['pointage'] as Map?)?.cast<String, dynamic>() ?? {});
  }

  Future<Statistiques> statistiques({String? mois}) async {
    final corps = await _get('/api/pointages/stats', {if (mois != null) 'mois': mois});
    return Statistiques.depuisJson(
        (corps['stats'] as Map?)?.cast<String, dynamic>() ?? {});
  }

  // -------------------------------------------------------------------------
  // Emploi du temps
  // -------------------------------------------------------------------------

  Future<List<Cours>> programmeDuJour() async {
    final corps = await _get('/api/programme/today');
    return ((corps['programme'] as List?) ?? [])
        .map((e) => Cours.depuisJson((e as Map).cast<String, dynamic>()))
        .toList();
  }

  Future<Map<String, List<Cours>>> programmeSemaine() async {
    final corps = await _get('/api/programme/week');
    final semaine = (corps['semaine'] as Map?)?.cast<String, dynamic>() ?? {};
    return semaine.map((jour, cours) => MapEntry(
          jour,
          ((cours as List?) ?? [])
              .map((e) => Cours.depuisJson((e as Map).cast<String, dynamic>()))
              .toList(),
        ));
  }

  // -------------------------------------------------------------------------
  // Notifications
  // -------------------------------------------------------------------------

  Future<List<Notification>> notifications() async {
    final corps = await _get('/api/notifications');
    return ((corps['notifications'] as List?) ?? [])
        .map((e) => Notification.depuisJson((e as Map).cast<String, dynamic>()))
        .toList();
  }

  Future<void> marquerLue(int id) => _envoyer('PUT', '/api/notifications/$id/read');

  /// Enregistre le jeton Firebase du téléphone pour les notifications push.
  Future<void> enregistrerAppareil(String jetonFcm, String plateforme) =>
      _envoyer('POST', '/api/me/device-token', {
        'token': jetonFcm,
        'plateforme': plateforme,
      });
}
