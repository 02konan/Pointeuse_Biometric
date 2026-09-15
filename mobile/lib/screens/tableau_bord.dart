import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../models/modeles.dart';
import '../services/api.dart';
import '../services/session.dart';
import '../theme.dart';
import '../widgets/communs.dart';
import '../widgets/graphiques.dart';
import 'notifications.dart';

/// Tableau de bord : en-tête de marque, statut du jour, assiduité du mois,
/// heures des sept derniers jours et cours du jour.
class TableauBordEcran extends StatefulWidget {
  const TableauBordEcran({super.key});

  @override
  State<TableauBordEcran> createState() => _TableauBordEcranState();
}

class _DonneesBord {
  const _DonneesBord(this.jour, this.stats, this.cours, this.historique);
  final Pointage jour;
  final Statistiques stats;
  final List<Cours> cours;
  final List<Pointage> historique;
}

class _TableauBordEcranState extends State<TableauBordEcran> {
  late Future<_DonneesBord> _donnees;

  @override
  void initState() {
    super.initState();
    _donnees = _charger();
  }

  Future<_DonneesBord> _charger() async {
    final api = context.read<Session>().api;
    final resultats = await Future.wait([
      api.pointageDuJour(),
      api.statistiques(),
      api.programmeDuJour(),
      api.pointages(limite: 7),
    ]);
    return _DonneesBord(
      resultats[0] as Pointage,
      resultats[1] as Statistiques,
      resultats[2] as List<Cours>,
      resultats[3] as List<Pointage>,
    );
  }

  /// Relance les appels ; l'erreur éventuelle est rendue par le FutureBuilder,
  /// pas remontée ici (sinon le RefreshIndicator resterait bloqué).
  Future<void> _rafraichir() async {
    setState(() => _donnees = _charger());
    try {
      await _donnees;
    } catch (_) {
      // affiché par le FutureBuilder
    }
  }

  @override
  Widget build(BuildContext context) {
    final session = context.watch<Session>();
    final nom = session.profil?.nom ?? session.utilisateur?.nom ?? '';

    return RefreshIndicator(
      onRefresh: _rafraichir,
      child: FutureBuilder<_DonneesBord>(
        future: _donnees,
        builder: (context, snapshot) {
          return ListView(
            padding: EdgeInsets.zero,
            children: [
              _EnTeteMarque(nom: nom, pointage: snapshot.data?.jour),
              Padding(
                padding: const EdgeInsets.fromLTRB(16, 0, 16, 28),
                child: switch (snapshot.connectionState) {
                  ConnectionState.waiting => const Padding(
                      padding: EdgeInsets.only(top: 60),
                      child: Center(child: CircularProgressIndicator()),
                    ),
                  _ when snapshot.hasError => Padding(
                      padding: const EdgeInsets.only(top: 40),
                      child: EtatErreur(
                        message: snapshot.error is ErreurApi
                            ? (snapshot.error as ErreurApi).message
                            : 'Chargement impossible',
                        onReessayer: _rafraichir,
                      ),
                    ),
                  _ => _Contenu(donnees: snapshot.data!),
                },
              ),
            ],
          );
        },
      ),
    );
  }
}

/// En-tête dégradé : la signature de l'application.
class _EnTeteMarque extends StatelessWidget {
  const _EnTeteMarque({required this.nom, this.pointage});

  final String nom;
  final Pointage? pointage;

  @override
  Widget build(BuildContext context) {
    final aujourdhui = DateTime.now();
    const mois = [
      'janvier', 'février', 'mars', 'avril', 'mai', 'juin',
      'juillet', 'août', 'septembre', 'octobre', 'novembre', 'décembre',
    ];

    return Stack(
      clipBehavior: Clip.none,
      children: [
        Container(
          padding: EdgeInsets.fromLTRB(
              20, MediaQuery.of(context).padding.top + 16, 20, 56),
          decoration: const BoxDecoration(
            gradient: Charte.enTete,
            borderRadius: BorderRadius.vertical(bottom: Radius.circular(28)),
          ),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Row(
                children: [
                  Expanded(
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(
                          '${aujourdhui.day} ${mois[aujourdhui.month - 1]}',
                          style: TextStyle(
                            color: Colors.white.withOpacity(0.72),
                            fontSize: 12,
                            fontWeight: FontWeight.w600,
                            letterSpacing: 0.5,
                          ),
                        ),
                        const SizedBox(height: 4),
                        Text(
                          nom.isEmpty ? 'Bonjour' : 'Bonjour $nom',
                          style: const TextStyle(
                            color: Colors.white,
                            fontSize: 23,
                            fontWeight: FontWeight.w700,
                            letterSpacing: -0.5,
                          ),
                        ),
                      ],
                    ),
                  ),
                  IconButton(
                    tooltip: 'Notifications',
                    onPressed: () => Navigator.of(context).push(
                      MaterialPageRoute(
                          builder: (_) => const NotificationsEcran()),
                    ),
                    icon: const Icon(Icons.notifications_none,
                        color: Colors.white, size: 26),
                  ),
                ],
              ),
            ],
          ),
        ),

        // La carte de statut chevauche le dégradé : c'est ce qui donne
        // sa profondeur à l'écran.
        Positioned(
          left: 16,
          right: 16,
          bottom: -44,
          child: _CarteStatut(pointage: pointage),
        ),
      ],
    );
  }
}

class _CarteStatut extends StatelessWidget {
  const _CarteStatut({this.pointage});

  final Pointage? pointage;

  @override
  Widget build(BuildContext context) {
    final (libelle, etat, icone) = switch (pointage?.statut) {
      'termine' => ('Journée terminée', 'succes', Icons.task_alt),
      'present' => ('En cours', 'alerte', Icons.login),
      'absent' => ('Aucun pointage', 'danger', Icons.remove_circle_outline),
      _ => ('Chargement…', 'neutre', Icons.hourglass_empty),
    };

    return Container(
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: context.surfaceCarte,
        borderRadius: BorderRadius.circular(18),
        border: Border.all(color: context.filet),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withOpacity(context.estSombre ? 0.4 : 0.07),
            blurRadius: 20,
            offset: const Offset(0, 6),
          ),
        ],
      ),
      child: Column(
        children: [
          Row(
            children: [
              Expanded(
                child: PastilleStatut(
                    etat: etat, libelle: libelle, icone: icone),
              ),
              if (pointage != null)
                Text(
                  pointage!.date,
                  style: TextStyle(fontSize: 11.5, color: context.encreDouce),
                ),
            ],
          ),
          const SizedBox(height: 14),
          Row(
            children: [
              _Horaire(
                libelle: 'Arrivée',
                valeur: Pointage.heure(pointage?.entree),
                icone: Icons.arrow_downward,
                etat: 'succes',
              ),
              const _Separateur(),
              _Horaire(
                libelle: 'Départ',
                valeur: Pointage.heure(pointage?.sortie),
                icone: Icons.arrow_upward,
                etat: 'danger',
              ),
              const _Separateur(),
              _Horaire(
                libelle: 'Durée',
                valeur: Pointage.heure(pointage?.duree),
                icone: Icons.timelapse,
                etat: 'neutre',
              ),
            ],
          ),
        ],
      ),
    );
  }
}

class _Separateur extends StatelessWidget {
  const _Separateur();

  @override
  Widget build(BuildContext context) =>
      Container(width: 1, height: 34, color: context.filet);
}

class _Horaire extends StatelessWidget {
  const _Horaire({
    required this.libelle,
    required this.valeur,
    required this.icone,
    required this.etat,
  });

  final String libelle;
  final String valeur;
  final IconData icone;
  final String etat;

  @override
  Widget build(BuildContext context) {
    return Expanded(
      child: Column(
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(icone, size: 12, color: Charte.statut(context, etat)),
              const SizedBox(width: 4),
              Text(
                libelle,
                style: TextStyle(
                  fontSize: 10.5,
                  fontWeight: FontWeight.w600,
                  letterSpacing: 0.4,
                  color: context.encreDouce,
                ),
              ),
            ],
          ),
          const SizedBox(height: 3),
          Text(
            valeur,
            style: const TextStyle(fontSize: 18, fontWeight: FontWeight.w700),
          ),
        ],
      ),
    );
  }
}

class _Contenu extends StatelessWidget {
  const _Contenu({required this.donnees});

  final _DonneesBord donnees;

  @override
  Widget build(BuildContext context) {
    final stats = donnees.stats;
    final taux = stats.joursPointes == 0
        ? 0.0
        : stats.joursComplets / stats.joursPointes;

    // Les journées arrivent de la plus récente à la plus ancienne.
    final barres = donnees.historique.reversed
        .map((p) => BarreJour(
              libelle: p.initialeJour,
              heures: p.heures,
              complet: p.complet,
            ))
        .toList();

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        const SizedBox(height: 56),

        const TitreSection('Ce mois-ci'),
        CarteApp(
          child: Row(
            children: [
              AnneauTaux(taux: taux, libelle: '% ASSIDUITÉ'),
              const SizedBox(width: 18),
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    _Mesure(
                      valeur: '${stats.joursComplets}',
                      libelle: 'journées complètes',
                      etat: 'succes',
                      icone: Icons.check_circle_outline,
                    ),
                    const SizedBox(height: 14),
                    _Mesure(
                      valeur: '${stats.joursIncomplets}',
                      libelle: 'départs manquants',
                      etat: 'alerte',
                      icone: Icons.error_outline,
                    ),
                    const SizedBox(height: 14),
                    _Mesure(
                      valeur: '${stats.heuresCumulees}h',
                      libelle: 'heures cumulées',
                      etat: 'neutre',
                      icone: Icons.schedule,
                    ),
                  ],
                ),
              ),
            ],
          ),
        ),

        const TitreSection('Heures par jour'),
        CarteApp(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text(
                'Sept derniers jours pointés',
                style: TextStyle(fontSize: 12.5, color: context.encreDouce),
              ),
              const SizedBox(height: 14),
              BarresHeures(jours: barres),
              if (barres.any((b) => !b.complet)) ...[
                const SizedBox(height: 10),
                Row(
                  children: [
                    Icon(Icons.error_outline,
                        size: 13, color: Charte.statut(context, 'alerte')),
                    const SizedBox(width: 6),
                    Expanded(
                      child: Text(
                        'Les barres hachurées sont des journées sans départ '
                        'enregistré : la durée réelle est inconnue.',
                        style: TextStyle(
                            fontSize: 11, color: context.encreDouce, height: 1.35),
                      ),
                    ),
                  ],
                ),
              ],
            ],
          ),
        ),

        TitreSection(
          "Cours d'aujourd'hui",
          action: donnees.cours.isEmpty
              ? null
              : Text(
                  '${donnees.cours.length}',
                  style: TextStyle(
                    fontSize: 11,
                    fontWeight: FontWeight.w700,
                    color: context.encreDouce,
                  ),
                ),
        ),
        if (donnees.cours.isEmpty)
          CarteApp(
            padding: const EdgeInsets.symmetric(vertical: 26),
            child: const EtatVide(
              icone: Icons.event_busy,
              message: 'Aucun cours prévu aujourd’hui',
            ),
          )
        else
          ...donnees.cours.map(
            (c) => Padding(
              padding: const EdgeInsets.only(bottom: 10),
              child: CarteApp(
                padding: const EdgeInsets.all(13),
                child: Row(
                  children: [
                    Container(
                      width: 3,
                      height: 38,
                      decoration: BoxDecoration(
                        color: Charte.primaire,
                        borderRadius: BorderRadius.circular(3),
                      ),
                    ),
                    const SizedBox(width: 13),
                    Expanded(
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Text(
                            c.matiere ?? 'Cours',
                            style: const TextStyle(
                                fontSize: 14.5, fontWeight: FontWeight.w600),
                          ),
                          const SizedBox(height: 2),
                          Text(
                            '${Pointage.heure(c.heureArrivee)} – '
                            '${Pointage.heure(c.heureDepart)}'
                            '${c.section == null ? '' : ' · ${c.section}'}',
                            style: TextStyle(
                                fontSize: 12, color: context.encreDouce),
                          ),
                        ],
                      ),
                    ),
                    if (c.type != null)
                      Container(
                        padding: const EdgeInsets.symmetric(
                            horizontal: 9, vertical: 4),
                        decoration: BoxDecoration(
                          color: Charte.primaire.withOpacity(0.11),
                          borderRadius: BorderRadius.circular(999),
                        ),
                        child: Text(
                          c.type!,
                          style: const TextStyle(
                              fontSize: 10.5, fontWeight: FontWeight.w600),
                        ),
                      ),
                  ],
                ),
              ),
            ),
          ),
      ],
    );
  }
}

class _Mesure extends StatelessWidget {
  const _Mesure({
    required this.valeur,
    required this.libelle,
    required this.etat,
    required this.icone,
  });

  final String valeur;
  final String libelle;
  final String etat;
  final IconData icone;

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        Icon(icone, size: 15, color: Charte.statut(context, etat)),
        const SizedBox(width: 9),
        Text(valeur,
            style: const TextStyle(fontSize: 17, fontWeight: FontWeight.w700)),
        const SizedBox(width: 6),
        Expanded(
          child: Text(
            libelle,
            style: TextStyle(fontSize: 11.5, color: context.encreDouce),
          ),
        ),
      ],
    );
  }
}
