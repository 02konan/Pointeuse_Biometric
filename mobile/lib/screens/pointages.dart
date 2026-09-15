import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../models/modeles.dart';
import '../services/api.dart';
import '../services/session.dart';
import '../theme.dart';
import '../widgets/communs.dart';

/// Historique des pointages, regroupés par journée.
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

  void _effacerPeriode() {
    setState(() {
      _periode = null;
      _pointages = _charger();
    });
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        Padding(
          padding: const EdgeInsets.fromLTRB(16, 12, 16, 4),
          child: Row(
            children: [
              Expanded(
                child: OutlinedButton.icon(
                  onPressed: _choisirPeriode,
                  icon: const Icon(Icons.date_range),
                  label: Text(_periode == null
                      ? 'Toutes les dates'
                      : '${_periode!.start.day}/${_periode!.start.month} – '
                          '${_periode!.end.day}/${_periode!.end.month}'),
                ),
              ),
              if (_periode != null)
                IconButton(
                  tooltip: 'Effacer le filtre',
                  onPressed: _effacerPeriode,
                  icon: const Icon(Icons.close),
                ),
            ],
          ),
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

                return ListView.separated(
                  padding: const EdgeInsets.fromLTRB(16, 8, 16, 24),
                  itemCount: pointages.length,
                  separatorBuilder: (_, __) => const SizedBox(height: 8),
                  itemBuilder: (context, i) => _CartePointage(pointages[i]),
                );
              },
            ),
          ),
        ),
      ],
    );
  }
}

class _CartePointage extends StatelessWidget {
  const _CartePointage(this.pointage);

  final Pointage pointage;

  @override
  Widget build(BuildContext context) {
    final complet = pointage.complet;
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(14),
        child: Row(
          children: [
            Container(
              width: 4,
              height: 42,
              decoration: BoxDecoration(
                color: complet ? Charte.succes : Charte.alerte,
                borderRadius: BorderRadius.circular(4),
              ),
            ),
            const SizedBox(width: 14),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(pointage.date,
                      style: const TextStyle(fontWeight: FontWeight.bold)),
                  const SizedBox(height: 2),
                  Text(
                    complet
                        ? '${Pointage.heure(pointage.entree)} → '
                            '${Pointage.heure(pointage.sortie)}'
                        : 'Entrée ${Pointage.heure(pointage.entree)} · départ non enregistré',
                    style: Theme.of(context).textTheme.bodySmall,
                  ),
                ],
              ),
            ),
            if (complet)
              Text(Pointage.heure(pointage.duree),
                  style: const TextStyle(
                      fontWeight: FontWeight.bold, color: Charte.primaire))
            else
              const Icon(Icons.warning_amber_rounded,
                  color: Charte.alerte, size: 20),
          ],
        ),
      ),
    );
  }
}
