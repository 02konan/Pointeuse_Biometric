import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../services/api.dart';
import '../services/session.dart';
import '../theme.dart';
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
      padding: EdgeInsets.zero,
      children: [
        // En-tête dégradé, identique à celui du tableau de bord.
        Container(
          width: double.infinity,
          padding: EdgeInsets.fromLTRB(
              20, MediaQuery.of(context).padding.top + 22, 20, 28),
          decoration: const BoxDecoration(
            gradient: Charte.enTete,
            borderRadius: BorderRadius.vertical(bottom: Radius.circular(28)),
          ),
          child: Column(
            children: [
              Container(
                width: 84,
                height: 84,
                decoration: BoxDecoration(
                  color: Colors.white.withOpacity(0.18),
                  shape: BoxShape.circle,
                  border: Border.all(color: Colors.white.withOpacity(0.35), width: 2),
                ),
                child: Center(
                  child: Text(
                    _initiale(profil?.nom ?? utilisateur?.nom),
                    style: const TextStyle(
                      fontSize: 34,
                      fontWeight: FontWeight.w700,
                      color: Colors.white,
                    ),
                  ),
                ),
              ),
              const SizedBox(height: 14),
              Text(
                profil?.nom ?? utilisateur?.nom ?? '',
                style: const TextStyle(
                  fontSize: 21,
                  fontWeight: FontWeight.w700,
                  color: Colors.white,
                  letterSpacing: -0.3,
                ),
              ),
              if (utilisateur != null) ...[
                const SizedBox(height: 8),
                Container(
                  padding:
                      const EdgeInsets.symmetric(horizontal: 12, vertical: 5),
                  decoration: BoxDecoration(
                    color: Colors.white.withOpacity(0.18),
                    borderRadius: BorderRadius.circular(999),
                  ),
                  child: Text(
                    utilisateur.role,
                    style: const TextStyle(
                      fontSize: 12,
                      fontWeight: FontWeight.w600,
                      color: Colors.white,
                    ),
                  ),
                ),
              ],
            ],
          ),
        ),
        const SizedBox(height: 22),

        Padding(
          padding: const EdgeInsets.fromLTRB(16, 0, 16, 28),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              if (profil != null) ...[
                const TitreSection('Mes informations'),
                CarteApp(
                  padding: const EdgeInsets.symmetric(horizontal: 16),
                  child: Column(
                    children: [
                      _Ligne(icone: Icons.badge_outlined, libelle: 'Matricule', valeur: profil.matricule),
                      _Ligne(icone: Icons.work_outline, libelle: 'Poste', valeur: profil.poste),
                      _Ligne(icone: Icons.apartment_outlined, libelle: 'Section', valeur: profil.section),
                      _Ligne(icone: Icons.phone_outlined, libelle: 'Téléphone', valeur: profil.telephone),
                      _Ligne(icone: Icons.mail_outline, libelle: 'Email', valeur: profil.email),
                      _Ligne(icone: Icons.home_outlined, libelle: 'Adresse', valeur: profil.adresse, dernier: true),
                    ],
                  ),
                ),
                const SizedBox(height: 14),
                OutlinedButton.icon(
                  onPressed: () => _modifierCoordonnees(context),
                  icon: const Icon(Icons.edit_outlined, size: 18),
                  label: const Text('Modifier mes coordonnées'),
                  style: OutlinedButton.styleFrom(
                    minimumSize: const Size.fromHeight(46),
                    shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(12)),
                  ),
                ),
              ] else
                CarteApp(
                  padding: const EdgeInsets.symmetric(vertical: 26),
                  child: const EtatVide(
                    icone: Icons.person_outline,
                    message: 'Aucune fiche employé associée à ce compte',
                  ),
                ),

              if (utilisateur?.type == 'utilisateur') ...[
                const SizedBox(height: 10),
                OutlinedButton.icon(
                  onPressed: () => _changerMotDePasse(context),
                  icon: const Icon(Icons.lock_outline, size: 18),
                  label: const Text('Changer mon mot de passe'),
                  style: OutlinedButton.styleFrom(
                    minimumSize: const Size.fromHeight(46),
                    shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(12)),
                  ),
                ),
              ],

              const SizedBox(height: 26),
              FilledButton.tonalIcon(
                onPressed: () => _confirmerDeconnexion(context),
                icon: const Icon(Icons.logout, size: 18),
                label: const Text('Déconnexion'),
                style: FilledButton.styleFrom(
                  minimumSize: const Size.fromHeight(48),
                  shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(12)),
                ),
              ),
            ],
          ),
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
  const _Ligne({
    required this.icone,
    required this.libelle,
    this.valeur,
    this.dernier = false,
  });

  final IconData icone;
  final String libelle;
  final String? valeur;
  final bool dernier;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(vertical: 11),
      decoration: dernier
          ? null
          : BoxDecoration(
              border: Border(bottom: BorderSide(color: context.filet)),
            ),
      child: Row(
        children: [
          Icon(icone, size: 18, color: context.encreDouce),
          const SizedBox(width: 13),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  libelle,
                  style: TextStyle(fontSize: 11, color: context.encreDouce),
                ),
                const SizedBox(height: 1),
                Text(
                  (valeur == null || valeur!.isEmpty) ? '—' : valeur!,
                  style: const TextStyle(
                      fontSize: 13.5, fontWeight: FontWeight.w600),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}
