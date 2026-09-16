import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../models/modeles.dart';
import '../services/api.dart';
import '../services/session.dart';
import '../theme.dart';
import '../widgets/communs.dart';

/// Emploi du temps de la semaine, un onglet par jour.
///
/// Chaque cours est présenté comme une plage horaire : l'heure de début est
/// la donnée que l'on cherche du regard, elle passe donc en tête de ligne.
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
        final aujourdhui = DateTime.now().weekday - 1;
        final total = semaine.values.fold<int>(0, (s, l) => s + l.length);

        return DefaultTabController(
          length: _jours.length,
          initialIndex: aujourdhui.clamp(0, _jours.length - 1),
          child: Column(
            children: [
              Container(
                color: context.surfaceCarte,
                child: TabBar(
                  isScrollable: true,
                  dividerColor: context.filet,
                  labelColor: Charte.primaire,
                  unselectedLabelColor: context.encreDouce,
                  indicatorColor: Charte.primaire,
                  indicatorWeight: 2.5,
                  labelStyle: const TextStyle(
                      fontSize: 13.5, fontWeight: FontWeight.w700),
                  unselectedLabelStyle: const TextStyle(
                      fontSize: 13.5, fontWeight: FontWeight.w500),
                  tabs: [
                    for (var i = 0; i < _jours.length; i++)
                      Tab(
                        child: Row(
                          children: [
                            Text(_jours[i]),
                            if ((semaine[_jours[i]] ?? []).isNotEmpty) ...[
                              const SizedBox(width: 6),
                              Container(
                                padding: const EdgeInsets.symmetric(
                                    horizontal: 6, vertical: 1),
                                decoration: BoxDecoration(
                                  color: Charte.primaire.withOpacity(0.13),
                                  borderRadius: BorderRadius.circular(999),
                                ),
                                child: Text(
                                  '${semaine[_jours[i]]!.length}',
                                  style: const TextStyle(
                                    fontSize: 10.5,
                                    fontWeight: FontWeight.w700,
                                  ),
                                ),
                              ),
                            ],
                            if (i == aujourdhui) ...[
                              const SizedBox(width: 5),
                              Container(
                                width: 5,
                                height: 5,
                                decoration: const BoxDecoration(
                                  color: Charte.primaire,
                                  shape: BoxShape.circle,
                                ),
                              ),
                            ],
                          ],
                        ),
                      ),
                  ],
                ),
              ),
              if (total == 0)
                const Expanded(
                  child: EtatVide(
                    icone: Icons.event_busy,
                    message: 'Aucun cours dans votre emploi du temps',
                  ),
                )
              else
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
          icone: Icons.free_breakfast_outlined, message: 'Journée libre');
    }

    final total = cours.fold<double>(0, (s, c) => s + _heures(c.duree));

    return ListView(
      padding: const EdgeInsets.fromLTRB(16, 16, 16, 28),
      children: [
        Row(
          children: [
            Icon(Icons.schedule, size: 15, color: context.encreDouce),
            const SizedBox(width: 7),
            Text(
              '${cours.length} cours · ${total.toStringAsFixed(total == total.roundToDouble() ? 0 : 1)}h au total',
              style: TextStyle(fontSize: 12.5, color: context.encreDouce),
            ),
          ],
        ),
        const SizedBox(height: 14),
        for (var i = 0; i < cours.length; i++) ...[
          _CarteCours(cours: cours[i]),
          if (i < cours.length - 1) const SizedBox(height: 10),
        ],
      ],
    );
  }

  static double _heures(String? duree) {
    final parties = (duree ?? '').split(':');
    if (parties.length < 2) return 0;
    return (int.tryParse(parties[0]) ?? 0) + (int.tryParse(parties[1]) ?? 0) / 60;
  }
}

class _CarteCours extends StatelessWidget {
  const _CarteCours({required this.cours});

  final Cours cours;

  @override
  Widget build(BuildContext context) {
    return CarteApp(
      padding: const EdgeInsets.all(14),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          // Plage horaire, reliée par un trait : on lit la durée d'un coup d'œil.
          Column(
            children: [
              Text(
                Pointage.heure(cours.heureArrivee),
                style: const TextStyle(fontSize: 14, fontWeight: FontWeight.w700),
              ),
              Container(
                width: 2,
                height: 18,
                margin: const EdgeInsets.symmetric(vertical: 3),
                decoration: BoxDecoration(
                  color: Charte.primaire.withOpacity(0.3),
                  borderRadius: BorderRadius.circular(2),
                ),
              ),
              Text(
                Pointage.heure(cours.heureDepart),
                style: TextStyle(fontSize: 12.5, color: context.encreDouce),
              ),
            ],
          ),
          const SizedBox(width: 14),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  cours.matiere ?? 'Cours',
                  style: const TextStyle(
                      fontSize: 15, fontWeight: FontWeight.w600, height: 1.25),
                ),
                const SizedBox(height: 5),
                Wrap(
                  spacing: 8,
                  runSpacing: 4,
                  children: [
                    if (cours.section != null)
                      _Detail(icone: Icons.apartment, texte: cours.section!),
                    if (cours.duree != null)
                      _Detail(
                          icone: Icons.timelapse,
                          texte: Pointage.heure(cours.duree)),
                  ],
                ),
              ],
            ),
          ),
          if (cours.type != null)
            Container(
              padding: const EdgeInsets.symmetric(horizontal: 9, vertical: 4),
              decoration: BoxDecoration(
                color: Charte.primaire.withOpacity(0.11),
                borderRadius: BorderRadius.circular(999),
              ),
              child: Text(
                cours.type!,
                style: const TextStyle(
                    fontSize: 10.5, fontWeight: FontWeight.w600),
              ),
            ),
        ],
      ),
    );
  }
}

class _Detail extends StatelessWidget {
  const _Detail({required this.icone, required this.texte});

  final IconData icone;
  final String texte;

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Icon(icone, size: 12, color: context.encreDouce),
        const SizedBox(width: 4),
        Text(texte, style: TextStyle(fontSize: 12, color: context.encreDouce)),
      ],
    );
  }
}
