import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../config.dart';
import '../services/session.dart';

/// Écran de connexion : enseignant (matricule) ou administration
/// (nom + mot de passe), comme sur l'interface web.
class ConnexionEcran extends StatefulWidget {
  const ConnexionEcran({super.key});

  @override
  State<ConnexionEcran> createState() => _ConnexionEcranState();
}

class _ConnexionEcranState extends State<ConnexionEcran> {
  final _formulaire = GlobalKey<FormState>();
  final _matricule = TextEditingController();
  final _nom = TextEditingController();
  final _motDePasse = TextEditingController();

  bool _administration = false;
  bool _motDePasseVisible = false;

  @override
  void dispose() {
    _matricule.dispose();
    _nom.dispose();
    _motDePasse.dispose();
    super.dispose();
  }

  Future<void> _valider() async {
    if (!_formulaire.currentState!.validate()) return;
    final session = context.read<Session>();
    final ok = _administration
        ? await session.connexionAdministration(_nom.text.trim(), _motDePasse.text)
        : await session.connexionEnseignant(_matricule.text.trim());

    if (!mounted || ok) return;
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text(session.erreur ?? 'Connexion impossible')),
    );
  }

  @override
  Widget build(BuildContext context) {
    final session = context.watch<Session>();
    final theme = Theme.of(context);

    return Scaffold(
      body: SafeArea(
        child: Center(
          child: SingleChildScrollView(
            padding: const EdgeInsets.all(24),
            child: ConstrainedBox(
              constraints: const BoxConstraints(maxWidth: 420),
              child: Form(
                key: _formulaire,
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    Icon(Icons.fingerprint,
                        size: 68, color: theme.colorScheme.primary),
                    const SizedBox(height: 16),
                    Text('BiometricWeb',
                        textAlign: TextAlign.center,
                        style: theme.textTheme.headlineSmall
                            ?.copyWith(fontWeight: FontWeight.bold)),
                    const SizedBox(height: 4),
                    Text('Suivi de vos pointages',
                        textAlign: TextAlign.center,
                        style: theme.textTheme.bodyMedium
                            ?.copyWith(color: theme.colorScheme.outline)),
                    const SizedBox(height: 28),

                    SegmentedButton<bool>(
                      segments: const [
                        ButtonSegment(
                            value: false,
                            label: Text('Enseignant'),
                            icon: Icon(Icons.badge_outlined)),
                        ButtonSegment(
                            value: true,
                            label: Text('Administration'),
                            icon: Icon(Icons.admin_panel_settings_outlined)),
                      ],
                      selected: {_administration},
                      onSelectionChanged: (valeur) =>
                          setState(() => _administration = valeur.first),
                    ),
                    const SizedBox(height: 20),

                    if (!_administration)
                      TextFormField(
                        controller: _matricule,
                        textInputAction: TextInputAction.done,
                        decoration: const InputDecoration(
                          labelText: 'Matricule',
                          prefixIcon: Icon(Icons.badge_outlined),
                        ),
                        validator: (v) => (v == null || v.trim().isEmpty)
                            ? 'Saisissez votre matricule'
                            : null,
                        onFieldSubmitted: (_) => _valider(),
                      )
                    else ...[
                      TextFormField(
                        controller: _nom,
                        decoration: const InputDecoration(
                          labelText: "Nom d'utilisateur",
                          prefixIcon: Icon(Icons.person_outline),
                        ),
                        validator: (v) => (v == null || v.trim().isEmpty)
                            ? "Saisissez votre nom d'utilisateur"
                            : null,
                      ),
                      const SizedBox(height: 12),
                      TextFormField(
                        controller: _motDePasse,
                        obscureText: !_motDePasseVisible,
                        decoration: InputDecoration(
                          labelText: 'Mot de passe',
                          prefixIcon: const Icon(Icons.lock_outline),
                          suffixIcon: IconButton(
                            icon: Icon(_motDePasseVisible
                                ? Icons.visibility_off_outlined
                                : Icons.visibility_outlined),
                            onPressed: () => setState(
                                () => _motDePasseVisible = !_motDePasseVisible),
                          ),
                        ),
                        validator: (v) => (v == null || v.isEmpty)
                            ? 'Saisissez votre mot de passe'
                            : null,
                        onFieldSubmitted: (_) => _valider(),
                      ),
                    ],

                    const SizedBox(height: 20),
                    FilledButton(
                      onPressed: session.chargement ? null : _valider,
                      child: session.chargement
                          ? const SizedBox(
                              height: 20,
                              width: 20,
                              child: CircularProgressIndicator(strokeWidth: 2))
                          : const Text('Se connecter'),
                    ),

                    if (!Config.connexionSecurisee) ...[
                      const SizedBox(height: 20),
                      // Le plan impose HTTPS : on avertit au lieu de laisser
                      // croire que la liaison est protégée.
                      Row(
                        children: [
                          Icon(Icons.warning_amber_rounded,
                              size: 18, color: theme.colorScheme.error),
                          const SizedBox(width: 8),
                          Expanded(
                            child: Text(
                              'Connexion non chiffrée (HTTP). À réserver aux '
                              'tests sur réseau local.',
                              style: theme.textTheme.bodySmall
                                  ?.copyWith(color: theme.colorScheme.error),
                            ),
                          ),
                        ],
                      ),
                    ],
                  ],
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}
