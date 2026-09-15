import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../config.dart';
import '../services/session.dart';
import '../theme.dart';

/// Écran de connexion : enseignant (matricule) ou administration
/// (nom + mot de passe), comme sur l'interface web.
///
/// La carte de saisie est posée sur le dégradé de marque : c'est le même
/// dégradé qui coiffe le tableau de bord, l'application s'annonce dès
/// le premier écran.
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

    return Scaffold(
      body: Container(
        decoration: const BoxDecoration(gradient: Charte.enTete),
        child: SafeArea(
          child: Center(
            child: SingleChildScrollView(
              padding: const EdgeInsets.all(22),
              child: ConstrainedBox(
                constraints: const BoxConstraints(maxWidth: 420),
                child: Form(
                  key: _formulaire,
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.stretch,
                    children: [
                      _marque(),
                      const SizedBox(height: 26),
                      _carteSaisie(context, session),
                      if (!Config.connexionSecurisee) ...[
                        const SizedBox(height: 18),
                        _avertissementHttp(),
                      ],
                    ],
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }

  Widget _marque() {
    return Column(
      children: [
        Container(
          width: 74,
          height: 74,
          decoration: BoxDecoration(
            color: Colors.white.withOpacity(0.15),
            borderRadius: BorderRadius.circular(22),
          ),
          child: const Icon(Icons.fingerprint, size: 42, color: Colors.white),
        ),
        const SizedBox(height: 18),
        const Text(
          'BiometricWeb',
          textAlign: TextAlign.center,
          style: TextStyle(
            color: Colors.white,
            fontSize: 26,
            fontWeight: FontWeight.w800,
            letterSpacing: -0.5,
          ),
        ),
        const SizedBox(height: 6),
        Text(
          'Suivi de vos pointages',
          textAlign: TextAlign.center,
          style: TextStyle(
            color: Colors.white.withOpacity(0.75),
            fontSize: 13.5,
          ),
        ),
      ],
    );
  }

  Widget _carteSaisie(BuildContext context, Session session) {
    return Container(
      padding: const EdgeInsets.all(20),
      decoration: BoxDecoration(
        color: Theme.of(context).colorScheme.surface,
        borderRadius: BorderRadius.circular(22),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          SegmentedButton<bool>(
            segments: const [
              ButtonSegment(
                value: false,
                label: Text('Enseignant'),
                icon: Icon(Icons.badge_outlined),
              ),
              ButtonSegment(
                value: true,
                label: Text('Administration'),
                icon: Icon(Icons.admin_panel_settings_outlined),
              ),
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
                  onPressed: () =>
                      setState(() => _motDePasseVisible = !_motDePasseVisible),
                ),
              ),
              validator: (v) => (v == null || v.isEmpty)
                  ? 'Saisissez votre mot de passe'
                  : null,
              onFieldSubmitted: (_) => _valider(),
            ),
          ],

          const SizedBox(height: 22),
          FilledButton(
            onPressed: session.chargement ? null : _valider,
            child: session.chargement
                ? const SizedBox(
                    height: 20,
                    width: 20,
                    child: CircularProgressIndicator(strokeWidth: 2),
                  )
                : const Text('Se connecter'),
          ),
        ],
      ),
    );
  }

  Widget _avertissementHttp() {
    // Le plan impose HTTPS : on avertit au lieu de laisser croire que la
    // liaison est protégée.
    return Row(
      children: [
        const Icon(Icons.warning_amber_rounded, size: 18, color: Colors.white),
        const SizedBox(width: 8),
        Expanded(
          child: Text(
            'Connexion non chiffrée (HTTP). À réserver aux tests sur '
            'réseau local.',
            style: TextStyle(
              fontSize: 12,
              height: 1.35,
              color: Colors.white.withOpacity(0.88),
            ),
          ),
        ),
      ],
    );
  }
}
