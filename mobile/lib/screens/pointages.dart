import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../models/modeles.dart';
import '../services/api.dart';
import '../services/session.dart';
import '../theme.dart';
import '../widgets/communs.dart';
import '../widgets/graphiques.dart';

/// Historique des pointages présenté en frise : une colonne de repères relie
/// les journées, chacune portant son arrivée, son départ et sa durée.
class PointagesEcran extends StatefulWidget {
  const PointagesEcran({super.key});

  @override
  State<PointagesEcran> createState() => _PointagesEcranState();
}

class _PointagesEcranState extends State<PointagesEcran> {
  late Future<List<Pointage>> _pointages;
  DateTimeRange? _periode;

  @override
  void initState() {
    super.initState();
    _pointages = _charger();
  }

  Future<List<Pointage>> _charger() {
    String? iso(DateTime? d) =>
        d == null ? null : d.toIso8601String().split('T').first;
    return context.read<Session>().api.pointages(
          debut: iso(_periode?.start),
          fin: iso(_periode?.end),
        );
  }

  Future<void> _choisirPeriode() async {
    final maintenant = DateTime.now();
    final choix = await showDateRangePicker(
      context: context,
      firstDate: DateTime(maintenant.year - 2),
      lastDate: maintenant,
      initialDateRange: _periode,
      locale: const Locale('fr'),
    );
    if (choix == null) return;
    setState(() {
      _periode = choix;
      _pointages = _charger();
    });
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        _BarreFiltre(
          periode: _periode,
          onChoisir: _choisirPeriode,
          onEffacer: _periode == null
              ? null
              : () => setState(() {
                    _periode = null;
                    _pointages = _charger();
                  }),
        ),
        Expanded(
          child: RefreshIndicator(
            onRefresh: () async => setState(() => _pointages = _charger()),
            child: FutureBuilder<List<Pointage>>(
              future: _pointages,
              builder: (context, snapshot) {
                if (snapshot.connectionState == ConnectionState.waiting) {
                  return const Center(child: CircularProgressIndicator());
                }
                if (snapshot.hasError) {
                  return ListView(children: [
                    const SizedBox(height: 60),
                    EtatErreur(
                      message: snapshot.error is ErreurApi
                          ? (snapshot.error as ErreurApi).message
                          : 'Chargement impossible',
                      onReessayer: () =>
                          setState(() => _pointages = _charger()),
                    ),
                  ]);
                }

                final pointages = snapshot.data ?? [];
                if (pointages.isEmpty) {
                  return ListView(children: const [
                    SizedBox(height: 60),
                    EtatVide(
                        icone: Icons.inbox_outlined,
                        message: 'Aucun pointage sur cette période'),
                  ]);
                }

                final total = pointages.fold<double>(0, (s, p) => s + p.heures);
                final incomplets = pointages.where((p) => !p.complet).length;

                return ListView(
                  padding: const EdgeInsets.fromLTRB(16, 4, 16, 28),
                  children: [
                    _Bilan(
                      journees: pointages.length,
                      heures: total,
                      incomplets: incomplets,
                    ),
                    const TitreSection('Détail des journées'),
                    for (var i = 0; i < pointages.length; i++)
                      _LigneFrise(
                        pointage: pointages[i],
                        premiere: i == 0,
                        derniere: i == pointages.length - 1,
                      ),
                  ],
                );
              },
            ),
          ),
        ),
      ],
    );
  }
}

class _BarreFiltre extends StatelessWidget {
  const _BarreFiltre({
    required this.periode,
    required this.onChoisir,
    this.onEffacer,
  });

  final DateTimeRange? periode;
  final VoidCallback onChoisir;
  final VoidCallback? onEffacer;

  @override
  Widget build(BuildContext context) {
    final actif = periode != null;
    return Padding(
      padding: const EdgeInsets.fromLTRB(16, 12, 16, 4),
      child: Row(
        children: [
          Expanded(
            child: InkWell(
              onTap: onChoisir,
              borderRadius: BorderRadius.circular(12),
              child: Container(
                padding:
                    const EdgeInsets.symmetric(horizontal: 14, vertical: 12),
                decoration: BoxDecoration(
                  color: actif
                      ? Charte.primaire.withOpacity(0.10)
                      : context.surfaceCarte,
                  borderRadius: BorderRadius.circular(12),
                  border: Border.all(
                      color: actif ? Charte.primaire : context.filet),
                ),
                child: Row(
                  children: [
                    Icon(Icons.date_range,
                        size: 18,
                        color: actif ? Charte.primaire : context.encreDouce),
                    const SizedBox(width: 9),
                    Text(
                      actif
                          ? '${periode!.start.day}/${periode!.start.month} – '
                              '${periode!.end.day}/${periode!.end.month}'
                          : 'Toutes les dates',
                      style: TextStyle(
                        fontSize: 13.5,
                        fontWeight: FontWeight.w600,
                        color: actif
                            ? Charte.primaire
                            : context.couleurs.onSurface,
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ),
          if (onEffacer != null)
            IconButton(
              tooltip: 'Effacer le filtre',
              onPressed: onEffacer,
              icon: const Icon(Icons.close),
            ),
        ],
      ),
    );
  }
}

/// Trois chiffres de synthèse sur la période affichée.
class _Bilan extends StatelessWidget {
  const _Bilan({
    required this.journees,
    required this.heures,
    required this.incomplets,
  });

  final int journees;
  final double heures;
  final int incomplets;

  @override
  Widget build(BuildContext context) {
    return CarteApp(
      padding: const EdgeInsets.symmetric(vertical: 16),
      child: Row(
        children: [
          _Chiffre(valeur: '$journees', libelle: 'journées'),
          Container(width: 1, height: 32, color: context.filet),
          _Chiffre(valeur: heures.toStringAsFixed(1), libelle: 'heures'),
          Container(width: 1, height: 32, color: context.filet),
          _Chiffre(
            valeur: '$incomplets',
            libelle: 'incomplètes',
            etat: incomplets > 0 ? 'alerte' : null,
          ),
        ],
      ),
    );
  }
}

class _Chiffre extends StatelessWidget {
  const _Chiffre({required this.valeur, required this.libelle, this.etat});

  final String valeur;
  final String libelle;
  final String? etat;

  @override
  Widget build(BuildContext context) {
    return Expanded(
      child: Column(
        children: [
          Text(
            valeur,
            style: const TextStyle(fontSize: 22, fontWeight: FontWeight.w800),
          ),
          const SizedBox(height: 2),
          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              if (etat != null) ...[
                Icon(Icons.error_outline,
                    size: 11, color: Charte.statut(context, etat!)),
                const SizedBox(width: 4),
              ],
              Text(
                libelle,
                style: TextStyle(fontSize: 11, color: context.encreDouce),
              ),
            ],
          ),
        ],
      ),
    );
  }
}

/// Une journée sur la frise.
class _LigneFrise extends StatelessWidget {
  const _LigneFrise({
    required this.pointage,
    required this.premiere,
    required this.derniere,
  });

  final Pointage pointage;
  final bool premiere;
  final bool derniere;

  @override
  Widget build(BuildContext context) {
    final complet = pointage.complet;
    final etat = complet ? 'succes' : 'alerte';
    final couleur = Charte.statut(context, etat);

    return IntrinsicHeight(
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          // Colonne des repères
          SizedBox(
            width: 28,
            child: Column(
              children: [
                Container(
                  width: 2,
                  height: 10,
                  color: premiere ? Colors.transparent : context.filet,
                ),
                Container(
                  width: 13,
                  height: 13,
                  decoration: BoxDecoration(
                    color: context.surfaceCarte,
                    shape: BoxShape.circle,
                    border: Border.all(color: couleur, width: 3),
                  ),
                ),
                Expanded(
                  child: Container(
                    width: 2,
                    color: derniere ? Colors.transparent : context.filet,
                  ),
                ),
              ],
            ),
          ),
          Expanded(
            child: Padding(
              padding: const EdgeInsets.only(bottom: 10),
              child: CarteApp(
                padding: const EdgeInsets.all(13),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Row(
                      children: [
                        Expanded(
                          child: Text(
                            pointage.date,
                            style: const TextStyle(
                                fontSize: 14, fontWeight: FontWeight.w700),
                          ),
                        ),
                        if (complet)
                          Text(
                            Pointage.heure(pointage.duree),
                            style: TextStyle(
                              fontSize: 14,
                              fontWeight: FontWeight.w700,
                              color: Charte.primaire,
                            ),
                          ),
                      ],
                    ),
                    const SizedBox(height: 9),
                    if (complet)
                      Row(
                        children: [
                          _Borne(
                            icone: Icons.arrow_downward,
                            heure: Pointage.heure(pointage.entree),
                            etat: 'succes',
                          ),
                          const SizedBox(width: 8),
                          Expanded(
                            child: Container(height: 1, color: context.filet),
                          ),
                          const SizedBox(width: 8),
                          _Borne(
                            icone: Icons.arrow_upward,
                            heure: Pointage.heure(pointage.sortie),
                            etat: 'danger',
                          ),
                        ],
                      )
                    else
                      Row(
                        children: [
                          _Borne(
                            icone: Icons.arrow_downward,
                            heure: Pointage.heure(pointage.entree),
                            etat: 'succes',
                          ),
                          const SizedBox(width: 10),
                          const PastilleStatut(
                            etat: 'alerte',
                            libelle: 'Départ manquant',
                            icone: Icons.error_outline,
                          ),
                        ],
                      ),
                  ],
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class _Borne extends StatelessWidget {
  const _Borne({required this.icone, required this.heure, required this.etat});

  final IconData icone;
  final String heure;
  final String etat;

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Icon(icone, size: 13, color: Charte.statut(context, etat)),
        const SizedBox(width: 4),
        Text(
          heure,
          style: const TextStyle(fontSize: 13, fontWeight: FontWeight.w600),
        ),
      ],
    );
  }
}
