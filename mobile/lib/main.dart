import 'package:flutter/material.dart';
import 'package:flutter_localizations/flutter_localizations.dart';
import 'package:provider/provider.dart';

import 'screens/accueil.dart';
import 'screens/connexion.dart';
import 'services/api.dart';
import 'services/session.dart';
import 'theme.dart';

void main() {
  runApp(const BiometricApp());
}

class BiometricApp extends StatelessWidget {
  const BiometricApp({super.key});

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (_) => Session(ApiClient())..demarrer(),
      child: MaterialApp(
        title: 'BiometricWeb',
        debugShowCheckedModeBanner: false,
        theme: Charte.clair(),
        darkTheme: Charte.sombre(),
        locale: const Locale('fr'),
        supportedLocales: const [Locale('fr'), Locale('en')],
        localizationsDelegates: const [
          GlobalMaterialLocalizations.delegate,
          GlobalWidgetsLocalizations.delegate,
          GlobalCupertinoLocalizations.delegate,
        ],
        home: const _Routeur(),
      ),
    );
  }
}

/// Aiguille vers la connexion ou l'accueil selon l'état de la session.
class _Routeur extends StatelessWidget {
  const _Routeur();

  @override
  Widget build(BuildContext context) {
    final session = context.watch<Session>();

    // Au tout premier démarrage, on attend la restauration du jeton stocké.
    if (session.chargement && session.utilisateur == null) {
      return const Scaffold(body: Center(child: CircularProgressIndicator()));
    }
    return session.connecte ? const AccueilEcran() : const ConnexionEcran();
  }
}
