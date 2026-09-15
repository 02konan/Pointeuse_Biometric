import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../models/modeles.dart';
import '../services/api.dart';
import '../services/session.dart';
import '../widgets/communs.dart';

/// Emploi du temps de la semaine, un onglet par jour.
class ProgrammeEcran extends StatefulWidget {
  const ProgrammeEcran({super.key});

  @override
  State<ProgrammeEcran> createState() => _ProgrammeEcranState();
}

class _ProgrammeEcranState extends State<ProgrammeEcran> {
  late Future<Map<String, List<Cours>>> _semaine;

  static const _jours = [
    'Lundi', 'Mardi', 'Mercredi', 'Jeudi', 'Vendredi', 'Samedi', 'Dimanche',
  ];

  @override
  void initState() {
    super.initState();
    _semaine = context.read<Session>().api.programmeSemaine();
  }

  void _recharger() {
    setState(() => _semaine = context.read<Session>().api.programmeSemaine());
  }

  @override
  Widget build(BuildContext context) {
    return FutureBuilder<Map<String, List<Cours>>>(
      future: _semaine,
      builder: (context, snapshot) {
        if (snapshot.connectionState == ConnectionState.waiting) {
          return const Center(child: CircularProgressIndicator());
        }
        if (snapshot.hasError) {
          return EtatErreur(
            message: snapshot.error is ErreurApi
                ? (snapshot.error as ErreurApi).message
                : 'Chargement impossible',
            onReessayer: _recharger,
          );
        }

        final semaine = snapshot.data ?? {};
        // L'onglet ouvert par défaut est le jour courant.
        final indexInitial = DateTime.now().weekday - 1;

        return DefaultTabController(
          length: _jours.length,
          initialIndex: indexInitial.clamp(0, _jours.length - 1),
          child: Column(
            children: [
              TabBar(
                isScrollable: true,
                tabs: [
                  for (final jour in _jours)
                    Tab(
                      child: Row(
                        children: [
                          Text(jour),
                          if ((semaine[jour] ?? []).isNotEmpty) ...[
                            const SizedBox(width: 6),
                            Badge(label: Text('${semaine[jour]!.length}')),
                          ],
                        ],
                      ),
                    ),
                ],
              ),
              Expanded(
                child: TabBarView(
                  children: [
                    for (final jour in _jours)
                      _ListeCours(cours: semaine[jour] ?? const []),
                  ],
                ),
              ),
            ],
          ),
        );
      },
    );
  }
}

class _ListeCours extends StatelessWidget {
  const _ListeCours({required this.cours});

  final List<Cours> cours;

  @override
  Widget build(BuildContext context) {
    if (cours.isEmpty) {
      return const EtatVide(
          icone: Icons.event_busy, message: 'Aucun cours ce jour-là');
    }
    return ListView.separated(
      padding: const EdgeInsets.all(16),
      itemCount: cours.length,
      separatorBuilder: (_, __) => const SizedBox(height: 8),
      itemBuilder: (context, i) {
        final c = cours[i];
        return Card(
          child: ListTile(
            leading: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Text(Pointage.heure(c.heureArrivee),
                    style: const TextStyle(fontWeight: FontWeight.bold)),
                Text(Pointage.heure(c.heureDepart),
                    style: Theme.of(context).textTheme.bodySmall),
              ],
            ),
            title: Text(c.matiere ?? 'Cours'),
            subtitle: Text([
              if (c.section != null) c.section!,
              if (c.duree != null) 'Durée ${Pointage.heure(c.duree)}',
            ].join(' · ')),
            trailing: c.type == null ? null : Chip(label: Text(c.type!)),
          ),
        );
      },
    );
  }
}
