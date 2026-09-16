/// Configuration de l'application.
///
/// L'URL de l'API se définit à la compilation, sans toucher au code :
///   flutter run --dart-define=API_URL=https://pointage.ifsmedu.com
class Config {
  /// Adresse de l'API Flask.
  ///
  /// Valeur par défaut : 10.0.2.2 est l'alias de « localhost » vu depuis
  /// l'émulateur Android. Sur un téléphone réel, passer l'adresse du serveur.
  static const String apiUrl = String.fromEnvironment(
    'API_URL',
    defaultValue: 'http://10.0.2.2:5000',
  );

  /// Le plan impose HTTPS en production : on refuse silencieusement de
  /// considérer une URL en clair comme sûre (voir AccueilEcran).
  static bool get connexionSecurisee => apiUrl.startsWith('https://');
}
