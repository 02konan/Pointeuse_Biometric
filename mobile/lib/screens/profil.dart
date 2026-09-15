import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../services/api.dart';
import '../services/session.dart';
import '../widgets/communs.dart';

/// Profil de l'utilisateur : consultation, modification des coordonnées,
/// changement de mot de passe et déconnexion.
class ProfilEcran extends StatelessWidget {
  const ProfilEcran({super.key});

  /// Première lettre du nom, pour l'avatar.
  static String _initiale(String? nom) {
    final valeur = (nom ?? '').trim();
    return valeur.isEmpty ? '?' : valeur[0].toUpperCase();
  }

  @override
  Widget build(BuildContext context) {
    final session = context.watch<Session>();
    final profil = session.profil;
    final utilisateur = session.utilisateur;

    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        Center(
          child: Column(
            children: [
              CircleAvatar(
                radius: 40,
                child: Text(
                  _initiale(profil?.nom ?? utilisateur?.nom),
                  style: const TextStyle(fontSize: 30),
                ),
              ),
              const SizedBox(height: 12),
              Text(profil?.nom ?? utilisateur?.nom ?? '',
                  style: Theme.of(context)
                      .textTheme
                      .titleLarge
                      ?.copyWith(fontWeight: FontWeight.bold)),
              if (utilisateur != null)
                Chip(label: Text(utilisateur.role)),
            ],
          ),
        ),
        const SizedBox(height: 20),

        if (profil != null) ...[
          Card(
            child: Column(
              children: [
                _Ligne(icone: Icons.badge_outlined, libelle: 'Matricule', valeur: profil.matricule),
                _Ligne(icone: Icons.work_outline, libelle: 'Poste', valeur: profil.poste),
                _Ligne(icone: Icons.apartment_outlined, libelle: 'Section', valeur: profil.section),
                _Ligne(icone: Icons.phone_outlined, libelle: 'Téléphone', valeur: profil.telephone),
                _Ligne(icone: Icons.mail_outline, libelle: 'Email', valeur: profil.email),
                _Ligne(icone: Icons.home_outlined, libelle: 'Adresse', valeur: profil.adresse),
              ],
            ),
          ),
          const SizedBox(height: 12),
          OutlinedButton.icon(
            onPressed: () => _modifierCoordonnees(context),
            icon: const Icon(Icons.edit_outlined),
            label: const Text('Modifier mes coordonnées'),
          ),
        ] else
          const Card(
            child: Padding(
              padding: EdgeInsets.symmetric(vertical: 24),
              child: EtatVide(
                icone: Icons.person_outline,
                message: 'Aucune fiche employé associée à ce compte',
              ),
            ),
          ),

        if (utilisateur?.type == 'utilisateur') ...[
          const SizedBox(height: 8),
          OutlinedButton.icon(
            onPressed: () => _changerMotDePasse(context),
            icon: const Icon(Icons.lock_outline),
            label: const Text('Changer mon mot de passe'),
          ),
        ],

        const SizedBox(height: 24),
        FilledButton.tonalIcon(
          onPressed: () => _confirmerDeconnexion(context),
          icon: const Icon(Icons.logout),
          label: const Text('Déconnexion'),
        ),
      ],
    );
  }

  Future<void> _modifierCoordonnees(BuildContext context) async {
    final session = context.read<Session>();
    final profil = session.profil!;
    final telephone = TextEditingController(text: profil.telephone ?? '');
    final email = TextEditingController(text: profil.email ?? '');
    final adresse = TextEditingController(text: profil.adresse ?? '');

    final enregistrer = await showDialog<bool>(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Mes coordonnées'),
        content: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              TextField(
                controller: telephone,
                keyboardType: TextInputType.phone,
                decoration: const InputDecoration(labelText: 'Téléphone'),
              ),
              const SizedBox(height: 12),
              TextField(
                controller: email,
                keyboardType: TextInputType.emailAddress,
                decoration: const InputDecoration(labelText: 'Email'),
              ),
              const SizedBox(height: 12),
              TextField(
                controller: adresse,
                decoration: const InputDecoration(labelText: 'Adresse'),
              ),
            ],
          ),
        ),
        actions: [
          TextButton(
              onPressed: () => Navigator.pop(context, false),
              child: const Text('Annuler')),
          FilledButton(
              onPressed: () => Navigator.pop(context, true),
              child: const Text('Enregistrer')),
        ],
      ),
    );

    if (enregistrer != true || !context.mounted) return;

    try {
      final nouveau = await session.api.majProfil(
        telephone: telephone.text.trim(),
        email: email.text.trim(),
        adresse: adresse.text.trim(),
      );
      session.majProfil(nouveau);
      if (!context.mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(content: Text('Coordonnées mises à jour')));
    } on ErreurApi catch (e) {
      if (!context.mounted) return;
      ScaffoldMessenger.of(context)
          .showSnackBar(SnackBar(content: Text(e.message)));
    }
  }

  Future<void> _changerMotDePasse(BuildContext context) async {
    final session = context.read<Session>();
    final ancien = TextEditingController();
    final nouveau = TextEditingController();

    final valider = await showDialog<bool>(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Changer mon mot de passe'),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            TextField(
              controller: ancien,
              obscureText: true,
              decoration: const InputDecoration(labelText: 'Mot de passe actuel'),
            ),
            const SizedBox(height: 12),
            TextField(
              controller: nouveau,
              obscureText: true,
              decoration: const InputDecoration(
                labelText: 'Nouveau mot de passe',
                helperText: '8 caractères minimum',
              ),
            ),
          ],
        ),
        actions: [
          TextButton(
              onPressed: () => Navigator.pop(context, false),
              child: const Text('Annuler')),
          FilledButton(
              onPressed: () => Navigator.pop(context, true),
              child: const Text('Valider')),
        ],
      ),
    );

    if (valider != true || !context.mounted) return;

    try {
      await session.api.changerMotDePasse(ancien.text, nouveau.text);
      if (!context.mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(content: Text('Mot de passe modifié')));
    } on ErreurApi catch (e) {
      if (!context.mounted) return;
      ScaffoldMessenger.of(context)
          .showSnackBar(SnackBar(content: Text(e.message)));
    }
  }

  Future<void> _confirmerDeconnexion(BuildContext context) async {
    final session = context.read<Session>();
    final confirme = await showDialog<bool>(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Déconnexion'),
        content: const Text('Voulez-vous vraiment vous déconnecter ?'),
        actions: [
          TextButton(
              onPressed: () => Navigator.pop(context, false),
              child: const Text('Annuler')),
          FilledButton(
              onPressed: () => Navigator.pop(context, true),
              child: const Text('Déconnexion')),
        ],
      ),
    );
    if (confirme == true) await session.deconnexion();
  }
}

class _Ligne extends StatelessWidget {
  const _Ligne({required this.icone, required this.libelle, this.valeur});

  final IconData icone;
  final String libelle;
  final String? valeur;

  @override
  Widget build(BuildContext context) {
    return ListTile(
      dense: true,
      leading: Icon(icone, size: 20),
      title: Text(libelle, style: Theme.of(context).textTheme.bodySmall),
      subtitle: Text(
        (valeur == null || valeur!.isEmpty) ? '—' : valeur!,
        style: const TextStyle(fontWeight: FontWeight.w500),
      ),
    );
  }
}
