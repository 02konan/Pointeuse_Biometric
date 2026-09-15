import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../models/modeles.dart';
import '../services/api.dart';
import '../services/session.dart';
import '../theme.dart';
import '../widgets/communs.dart';

/// Tableau de bord : statut du jour et statistiques personnelles du mois.
class TableauBordEcran extends StatefulWidget {
  const TableauBordEcran({super.key});

  @override
  State<TableauBordEcran> createState() => _TableauBordEcranState();
}

class _TableauBordEcranState extends State<TableauBordEcran> {
  late Future<(Pointage, Statistiques, List<Cours>)> _donnees;

  @override
  void initState() {
    super.initState();
    _donnees = _charger();
  }

  Future<(Pointage, Statistiques, List<Cours>)> _charger() async {
    final api = context.read<Session>().api;
    final resultats = await Future.wait([
      api.pointageDuJour(),
      api.statistiques(),
      api.programmeDuJour(),
    ]);
    return (
      resultats[0] as Pointage,
      resultats[1] as Statistiques,
      resultats[2] as List<Cours>,
    );
  }

  Future<void> _rafraichir() async {
    setState(() => _donnees = _charger());
    await _donnees.catchError((_) => (
          const Pointage(date: '', nbPassages: 0, complet: false),
          const Statistiques(
              mois: '',
              joursPointes: 0,
              joursComplets: 0,
              joursIncomplets: 0,
              heuresCumulees: 0,
              dureeMoyenne: 0),
          <Cours>[],
        ));
  }

  @override
  Widget build(BuildContext context) {
    final session = context.watch<Session>();
    final nom = session.profil?.nom ?? session.utilisateur?.nom ?? '';

    return RefreshIndicator(
      onRefresh: _rafraichir,
      child: FutureBuilder<(Pointage, Statistiques, List<Cours>)>(
        future: _donnees,
        builder: (context, snapshot) {
          if (snapshot.connectionState == ConnectionState.waiting) {
            return const Center(child: CircularProgressIndicator());
          }
          if (snapshot.hasError) {
            return ListView(children: [
              const SizedBox(height: 80),
              EtatErreur(
                message: snapshot.error is ErreurApi
                    ? (snapshot.error as ErreurApi).message
                    : 'Chargement impossible',
                onReessayer: _rafraichir,
              ),
            ]);
          }

          final (pointage, stats, cours) = snapshot.data!;
          return ListView(
            padding: const EdgeInsets.all(16),
            children: [
              Text('Bonjour ${nom.isEmpty ? '' : nom}',
                  style: Theme.of(context)
                      .textTheme
                      .titleLarge
                      ?.copyWith(fontWeight: FontWeight.bold)),
              const SizedBox(height: 16),
              _CarteStatut(pointage: pointage),
              const SizedBox(height: 20),
              Text('Ce mois-ci',
                  style: Theme.of(context).textTheme.titleMedium),
              const SizedBox(height: 10),
              GridView.count(
                crossAxisCount: 2,
                shrinkWrap: true,
                physics: const NeverScrollableScrollPhysics(),
                childAspectRatio: 1.35,
                crossAxisSpacing: 12,
                mainAxisSpacing: 12,
                children: [
                  CarteStat(
                      libelle: 'Jours pointés',
                      valeur: '${stats.joursPointes}',
                      icone: Icons.event_available,
                      couleur: Charte.primaire),
                  CarteStat(
                      libelle: 'Journées complètes',
                      valeur: '${stats.joursComplets}',
                      icone: Icons.check_circle_outline,
                      couleur: Charte.succes),
                  CarteStat(
                      libelle: 'Journées incomplètes',
                      valeur: '${stats.joursIncomplets}',
                      icone: Icons.error_outline,
                      couleur: Charte.alerte),
                  CarteStat(
                      libelle: 'Heures cumulées',
                      valeur: '${stats.heuresCumulees}',
                      icone: Icons.schedule,
                      couleur: Charte.primaire),
                ],
              ),
              const SizedBox(height: 20),
              Text("Cours d'aujourd'hui",
                  style: Theme.of(context).textTheme.titleMedium),
              const SizedBox(height: 10),
              if (cours.isEmpty)
                const Card(
                  child: Padding(
                    padding: EdgeInsets.symmetric(vertical: 24),
                    child: EtatVide(
                        icone: Icons.event_busy,
                        message: 'Aucun cours prévu aujourd’hui'),
                  ),
                )
              else
                ...cours.map((c) => Padding(
                      padding: const EdgeInsets.only(bottom: 8),
                      child: _LigneCours(cours: c),
                    )),
            ],
          );
        },
      ),
    );
  }
}

class _CarteStatut extends StatelessWidget {
  const _CarteStatut({required this.pointage});

  final Pointage pointage;

  @override
  Widget build(BuildContext context) {
    final (libelle, couleur, icone) = switch (pointage.statut) {
      'termine' => ('Journée terminée', Charte.succes, Icons.task_alt),
      'present' => ('Présent', Charte.primaire, Icons.login),
      _ => ('Aucun pointage aujourd’hui', Charte.danger, Icons.do_not_disturb_on_outlined),
    };

    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          children: [
            Row(
              children: [
                Container(
                  padding: const EdgeInsets.all(10),
                  decoration: BoxDecoration(
                    color: couleur.withOpacity(0.12),
                    borderRadius: BorderRadius.circular(12),
                  ),
                  child: Icon(icone, color: couleur),
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(libelle,
                          style: Theme.of(context)
                              .textTheme
                              .titleMedium
                              ?.copyWith(fontWeight: FontWeight.bold)),
                      Text(pointage.date,
                          style: Theme.of(context).textTheme.bodySmall),
                    ],
                  ),
                ),
              ],
            ),
            const Divider(height: 26),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                _Horaire(
                    libelle: 'Arrivée',
                    valeur: Pointage.heure(pointage.entree),
                    couleur: Charte.succes),
                _Horaire(
                    libelle: 'Départ',
                    valeur: Pointage.heure(pointage.sortie),
                    couleur: Charte.danger),
                _Horaire(
                    libelle: 'Durée',
                    valeur: Pointage.heure(pointage.duree),
                    couleur: Charte.primaire),
              ],
            ),
          ],
        ),
      ),
    );
  }
}

class _Horaire extends StatelessWidget {
  const _Horaire({required this.libelle, required this.valeur, required this.couleur});

  final String libelle;
  final String valeur;
  final Color couleur;

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        Text(valeur,
            style: Theme.of(context)
                .textTheme
                .titleMedium
                ?.copyWith(fontWeight: FontWeight.bold, color: couleur)),
        Text(libelle, style: Theme.of(context).textTheme.bodySmall),
      ],
    );
  }
}

class _LigneCours extends StatelessWidget {
  const _LigneCours({required this.cours});

  final Cours cours;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: ListTile(
        leading: const CircleAvatar(child: Icon(Icons.menu_book_outlined, size: 20)),
        title: Text(cours.matiere ?? 'Cours'),
        subtitle: Text(
            '${Pointage.heure(cours.heureArrivee)} – ${Pointage.heure(cours.heureDepart)}'
            '${cours.section == null ? '' : ' · ${cours.section}'}'),
        trailing: cours.type == null ? null : Chip(label: Text(cours.type!)),
      ),
    );
  }
}
