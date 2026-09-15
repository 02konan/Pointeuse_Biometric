import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../services/session.dart';
import '../theme.dart';
import 'notifications.dart';
import 'pointages.dart';
import 'profil.dart';
import 'programme.dart';
import 'tableau_bord.dart';

/// Écran principal : navigation entre les quatre sections.
///
/// Le tableau de bord porte son propre en-tête dégradé et n'a donc pas de
/// barre d'application ; les autres onglets en gardent une, classique.
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
    // Un compte d'administration n'a ni pointages ni emploi du temps
    // personnels : seul le profil lui est proposé.
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
    // Tableau de bord et profil portent leur propre en-tête dégradé ;
    // seuls Pointages et Programme reçoivent une barre d'application.
    final avecBarre = enseignant && (index == 1 || index == 2);

    return Scaffold(
      appBar: !avecBarre
          ? null
          : AppBar(
              title: Text(_titres[index]),
              actions: [
                if (enseignant)
                  IconButton(
                    tooltip: 'Notifications',
                    icon: const Icon(Icons.notifications_none),
                    onPressed: () => Navigator.of(context).push(
                      MaterialPageRoute(
                          builder: (_) => const NotificationsEcran()),
                    ),
                  ),
              ],
            ),
      body: pages[index],
      bottomNavigationBar: enseignant
          ? NavigationBar(
              selectedIndex: index,
              height: 66,
              backgroundColor: context.surfaceCarte,
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
