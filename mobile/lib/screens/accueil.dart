import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../services/session.dart';
import 'notifications.dart';
import 'pointages.dart';
import 'profil.dart';
import 'programme.dart';
import 'tableau_bord.dart';

/// Écran principal : barre de navigation entre les quatre sections.
class AccueilEcran extends StatefulWidget {
  const AccueilEcran({super.key});

  @override
  State<AccueilEcran> createState() => _AccueilEcranState();
}

class _AccueilEcranState extends State<AccueilEcran> {
  int _onglet = 0;

  static const _titres = [
    'Tableau de bord',
    'Mes pointages',
    'Emploi du temps',
    'Mon profil',
  ];

  @override
  Widget build(BuildContext context) {
    final utilisateur = context.watch<Session>().utilisateur;
    // Les comptes d'administration n'ont ni pointages ni emploi du temps
    // personnels : on ne leur propose que le profil.
    final enseignant = utilisateur?.estEnseignant ?? false;

    final pages = enseignant
        ? const [
            TableauBordEcran(),
            PointagesEcran(),
            ProgrammeEcran(),
            ProfilEcran(),
          ]
        : const [ProfilEcran()];

    final index = _onglet.clamp(0, pages.length - 1);

    return Scaffold(
      appBar: AppBar(
        title: Text(enseignant ? _titres[index] : 'Mon profil'),
        actions: [
          if (enseignant)
            IconButton(
              tooltip: 'Notifications',
              icon: const Icon(Icons.notifications_none),
              onPressed: () => Navigator.of(context).push(
                MaterialPageRoute(builder: (_) => const NotificationsEcran()),
              ),
            ),
        ],
      ),
      body: pages[index],
      bottomNavigationBar: enseignant
          ? NavigationBar(
              selectedIndex: index,
              onDestinationSelected: (i) => setState(() => _onglet = i),
              destinations: const [
                NavigationDestination(
                    icon: Icon(Icons.dashboard_outlined),
                    selectedIcon: Icon(Icons.dashboard),
                    label: 'Accueil'),
                NavigationDestination(
                    icon: Icon(Icons.access_time),
                    selectedIcon: Icon(Icons.access_time_filled),
                    label: 'Pointages'),
                NavigationDestination(
                    icon: Icon(Icons.calendar_month_outlined),
                    selectedIcon: Icon(Icons.calendar_month),
                    label: 'Programme'),
                NavigationDestination(
                    icon: Icon(Icons.person_outline),
                    selectedIcon: Icon(Icons.person),
                    label: 'Profil'),
              ],
            )
          : null,
    );
  }
}
