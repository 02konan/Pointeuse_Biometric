# Application mobile — Pointage biométrique

Application Flutter de consultation des pointages pour les enseignants.
Elle ne lit **jamais** MySQL directement : toutes les données transitent par
l'API REST Flask (`App/routes.py`, préfixe `/api`).

```
Flutter  ──HTTPS──>  API Flask  ──>  MySQL
                         └──>  Firebase Cloud Messaging  ──>  téléphone
```

## 1. Générer les dossiers de plateforme

Le dépôt contient le code Dart (`lib/`) et `pubspec.yaml`, mais **pas** les
dossiers `android/` et `ios/` : ils sont générés par Flutter et dépendent de
votre machine. Première étape, depuis `mobile/` :

```bash
flutter create . --platforms=android,ios --project-name biometric_mobile
flutter pub get
```

## 2. Lancer l'application

L'adresse de l'API se passe à la compilation, sans modifier le code :

```bash
# Émulateur Android vers un Flask lancé sur le PC (10.0.2.2 = localhost de l'hôte)
flutter run --dart-define=API_URL=http://10.0.2.2:5000

# Téléphone réel sur le même réseau
flutter run --dart-define=API_URL=http://192.168.1.10:5000

# Production
flutter build apk --dart-define=API_URL=https://votre-domaine.example
```

En HTTP, Android bloque le trafic en clair depuis Android 9. Pour les tests
en réseau local uniquement, ajouter dans
`android/app/src/main/AndroidManifest.xml`, sur la balise `<application>` :

```xml
android:usesCleartextTraffic="true"
```

À retirer en production : le plan impose HTTPS. L'écran de connexion affiche
d'ailleurs un avertissement tant que l'URL n'est pas en `https://`.

## 3. Structure

| Fichier | Rôle |
|---|---|
| `lib/config.dart` | Adresse de l'API (`--dart-define=API_URL=…`) |
| `lib/services/api.dart` | Client HTTP, jeton stocké de façon sécurisée |
| `lib/services/session.dart` | État d'authentification partagé (`provider`) |
| `lib/models/modeles.dart` | Modèles JSON (profil, pointage, cours, notification) |
| `lib/screens/` | Connexion, tableau de bord, pointages, emploi du temps, notifications, profil |
| `lib/widgets/communs.dart` | États vides, erreurs réseau, tuiles de statistiques |

## 4. Connexion

Deux modes, comme sur l'interface web :

- **Enseignant** : matricule seul. C'est le fonctionnement actuel du serveur
  (`verification_prof`), qui n'exige aucun secret — toute personne connaissant
  un matricule accède aux données de l'enseignant. À renforcer avant une mise
  en production hors réseau interne.
- **Administration** : nom d'utilisateur + mot de passe.

Le jeton renvoyé par `/api/login` est conservé dans le stockage sécurisé du
système (Keychain / EncryptedSharedPreferences), jamais en clair, et vaut
7 jours. Aucun mot de passe n'est stocké sur le téléphone.

## 5. Notifications push

La partie serveur est prête (`POST /api/me/device-token`,
`GET /api/notifications`), mais **Firebase n'est pas encore branché** :

1. Jouer la migration `database/migrations/001_mobile.sql` (tables
   `notifications_mobile` et `appareils_mobiles`) ;
2. créer le projet Firebase et déposer `google-services.json` ;
3. ajouter `firebase_messaging` à `pubspec.yaml`, puis appeler
   `ApiClient.enregistrerAppareil()` avec le jeton FCM au démarrage ;
4. côté serveur, envoyer via FCM depuis `programme/api_mobile.creer_notification`.

En attendant, l'écran Notifications lit la table via l'API : les messages
s'affichent, mais sans alerte push.

## 6. Limite connue

Ce code **n'a pas été compilé** : l'environnement où il a été écrit ne dispose
pas du SDK Flutter. Les vérifications faites sont statiques (imports résolus,
structure équilibrée, correspondance de chaque appel avec une route Flask
existante). Prévoir un premier `flutter analyze` pour lever les avertissements
de style résiduels.
