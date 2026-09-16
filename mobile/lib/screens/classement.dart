import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../models/modeles.dart';
import '../services/api.dart';
import '../services/session.dart';
import '../theme.dart';
import '../widgets/communs.dart';

/// Classement de la section : podium, position de l'utilisateur et suite
/// du tableau.
///
/// Le rang seul ne motive personne : chaque écran affiche donc ce qu'il
/// manque concrètement pour gagner une place.
class ClassementEcran extends StatefulWidget {
  const ClassementEcran({super.key});

  @override
  State<ClassementEcran> createState() => _ClassementEcranState();
}

class _ClassementEcranState extends State<ClassementEcran> {
  late Future<Classement> _classement;

  @override
  void initState() {
    super.initState();
    _classement = context.read<Session>().api.classement();
  }

  void _recharger() {
    setState(() => _classement = context.read<Session>().api.classement());
  }

  @override
  Widget build(BuildContext context) {
    return RefreshIndicator(
      onRefresh: () async => _recharger(),
      child: FutureBuilder<Classement>(
        future: _classement,
        builder: (context, snapshot) {
          if (snapshot.connectionState == ConnectionState.waiting) {
            return const Center(child: CircularProgressIndicator());
          }
          if (snapshot.hasError) {
            return ListView(children: [
              const SizedBox(height: 70),
              EtatErreur(
                message: snapshot.error is ErreurApi
                    ? (snapshot.error as ErreurApi).message
                    : 'Chargement impossible',
                onReessayer: _recharger,
              ),
            ]);
          }

          final classement = snapshot.data!;
          if (classement.lignes.isEmpty) {
            return ListView(children: const [
              SizedBox(height: 70),
              EtatVide(
                icone: Icons.emoji_events_outlined,
                message: 'Aucun pointage ce mois-ci dans votre section :\n'
                    'le classement démarrera au premier passage.',
              ),
            ]);
          }

          return ListView(
            padding: EdgeInsets.zero,
            children: [
              _Podium(classement: classement),
              Padding(
                padding: const EdgeInsets.fromLTRB(16, 4, 16, 28),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    if (classement.moi != null)
                      _MaPosition(
                        moi: classement.moi!,
                        objectif: classement.objectif,
                        total: classement.total,
                      ),
                    if (classement.suite.isNotEmpty) ...[
                      const TitreSection('Suite du classement'),
                      for (final ligne in classement.suite)
                        Padding(
                          padding: const EdgeInsets.only(bottom: 8),
                          child: _LigneClassement(
                            ligne: ligne,
                            moi: ligne.matricule == classement.moi?.matricule,
                          ),
                        ),
                    ],
                    const SizedBox(height: 18),
                    _MentionMethode(),
                  ],
                ),
              ),
            ],
          );
        },
      ),
    );
  }
}

/// Podium : 2e, 1er, 3e — l'ordre de lecture d'un vrai podium.
class _Podium extends StatelessWidget {
  const _Podium({required this.classement});

  final Classement classement;

  static const _hauteurs = [76.0, 104.0, 58.0];

  @override
  Widget build(BuildContext context) {
    final podium = classement.podium;

    return Container(
      padding: EdgeInsets.fromLTRB(
          16, MediaQuery.of(context).padding.top + 18, 16, 22),
      decoration: const BoxDecoration(
        gradient: Charte.enTete,
        borderRadius: BorderRadius.vertical(bottom: Radius.circular(28)),
      ),
      child: Column(
        children: [
          Row(
            children: [
              const Icon(Icons.emoji_events, color: Colors.white, size: 22),
              const SizedBox(width: 9),
              Expanded(
                child: Text(
                  classement.section == null
                      ? 'Classement du mois'
                      : 'Section ${classement.section}',
                  style: const TextStyle(
                    color: Colors.white,
                    fontSize: 16,
                    fontWeight: FontWeight.w700,
                  ),
                ),
              ),
              Container(
                padding:
                    const EdgeInsets.symmetric(horizontal: 10, vertical: 4),
                decoration: BoxDecoration(
                  color: Colors.white.withOpacity(0.18),
                  borderRadius: BorderRadius.circular(999),
                ),
                child: Text(
                  classement.mois,
                  style: const TextStyle(
                    color: Colors.white,
                    fontSize: 11.5,
                    fontWeight: FontWeight.w600,
                  ),
                ),
              ),
            ],
          ),
          const SizedBox(height: 22),
          Row(
            crossAxisAlignment: CrossAxisAlignment.end,
            children: [
              for (var i = 0; i < 3; i++)
                Expanded(
                  child: _MarchePodium(
                    ligne: podium[i],
                    hauteur: _hauteurs[i],
                    moi: podium[i]?.matricule == classement.moi?.matricule,
                  ),
                ),
            ],
          ),
        ],
      ),
    );
  }
}

class _MarchePodium extends StatelessWidget {
  const _MarchePodium({
    required this.ligne,
    required this.hauteur,
    required this.moi,
  });

  final RangEnseignant? ligne;
  final double hauteur;
  final bool moi;

  /// Or, argent, bronze : la couleur double le numéro de rang, elle ne le
  /// remplace pas — le chiffre reste écrit sur la marche.
  static const _medailles = {
    1: Color(0xFFE0B341),
    2: Color(0xFFC5CBD8),
    3: Color(0xFFCB8B55),
  };

  @override
  Widget build(BuildContext context) {
    if (ligne == null) {
      return SizedBox(height: hauteur + 76);
    }
    final medaille = _medailles[ligne!.rang] ?? Colors.white;

    return Column(
      mainAxisAlignment: MainAxisAlignment.end,
      children: [
        Stack(
          clipBehavior: Clip.none,
          alignment: Alignment.center,
          children: [
            Container(
              width: ligne!.rang == 1 ? 60 : 50,
              height: ligne!.rang == 1 ? 60 : 50,
              decoration: BoxDecoration(
                color: Colors.white.withOpacity(0.2),
                shape: BoxShape.circle,
                border: Border.all(color: medaille, width: 2.5),
              ),
              child: Center(
                child: Text(
                  ligne!.initiale,
                  style: TextStyle(
                    color: Colors.white,
                    fontSize: ligne!.rang == 1 ? 24 : 20,
                    fontWeight: FontWeight.w700,
                  ),
                ),
              ),
            ),
            if (ligne!.rang == 1)
              const Positioned(
                top: -18,
                child: Icon(Icons.workspace_premium,
                    color: Color(0xFFE0B341), size: 24),
              ),
          ],
        ),
        const SizedBox(height: 7),
        Text(
          ligne!.nom,
          maxLines: 1,
          overflow: TextOverflow.ellipsis,
          style: TextStyle(
            color: Colors.white,
            fontSize: 12.5,
            fontWeight: moi ? FontWeight.w800 : FontWeight.w600,
          ),
        ),
        Text(
          '${ligne!.joursComplets} j · ${ligne!.heures.toStringAsFixed(0)} h',
          style: TextStyle(
            color: Colors.white.withOpacity(0.72),
            fontSize: 10.5,
          ),
        ),
        const SizedBox(height: 7),
        Container(
          height: hauteur,
          margin: const EdgeInsets.symmetric(horizontal: 4),
          decoration: BoxDecoration(
            color: Colors.white.withOpacity(0.16),
            borderRadius:
                const BorderRadius.vertical(top: Radius.circular(10)),
            border: Border(
              top: BorderSide(color: medaille, width: 3),
              left: BorderSide(color: Colors.white.withOpacity(0.12)),
              right: BorderSide(color: Colors.white.withOpacity(0.12)),
            ),
          ),
          child: Center(
            child: Text(
              '${ligne!.rang}',
              style: TextStyle(
                color: Colors.white.withOpacity(0.9),
                fontSize: 26,
                fontWeight: FontWeight.w800,
              ),
            ),
          ),
        ),
      ],
    );
  }
}

/// Carte « ma position » : le rang, puis l'objectif suivant.
class _MaPosition extends StatelessWidget {
  const _MaPosition({
    required this.moi,
    required this.total,
    this.objectif,
  });

  final RangEnseignant moi;
  final Objectif? objectif;
  final int total;

  @override
  Widget build(BuildContext context) {
    return Container(
      margin: const EdgeInsets.only(top: 18),
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: context.surfaceCarte,
        borderRadius: BorderRadius.circular(18),
        border: Border.all(color: Charte.primaire.withOpacity(0.45), width: 1.5),
      ),
      child: Column(
        children: [
          Row(
            children: [
              Container(
                width: 52,
                height: 52,
                decoration: BoxDecoration(
                  color: Charte.primaire.withOpacity(0.12),
                  borderRadius: BorderRadius.circular(15),
                ),
                child: Center(
                  child: Text(
                    '${moi.rang}',
                    style: const TextStyle(
                      fontSize: 24,
                      fontWeight: FontWeight.w800,
                      color: Charte.primaire,
                    ),
                  ),
                ),
              ),
              const SizedBox(width: 14),
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      'Votre position',
                      style:
                          TextStyle(fontSize: 11.5, color: context.encreDouce),
                    ),
                    const SizedBox(height: 2),
                    Text(
                      '${moi.rang}e sur $total',
                      style: const TextStyle(
                          fontSize: 18, fontWeight: FontWeight.w700),
                    ),
                  ],
                ),
              ),
              Column(
                crossAxisAlignment: CrossAxisAlignment.end,
                children: [
                  Text('${moi.joursComplets} journées',
                      style: const TextStyle(
                          fontSize: 13.5, fontWeight: FontWeight.w700)),
                  Text('${moi.heures.toStringAsFixed(1)} h cumulées',
                      style: TextStyle(
                          fontSize: 11.5, color: context.encreDouce)),
                ],
              ),
            ],
          ),
          if (objectif != null) ...[
            const SizedBox(height: 14),
            Container(
              padding: const EdgeInsets.symmetric(horizontal: 13, vertical: 11),
              decoration: BoxDecoration(
                color: Charte.primaire.withOpacity(0.08),
                borderRadius: BorderRadius.circular(12),
              ),
              child: Row(
                children: [
                  const Icon(Icons.trending_up,
                      size: 18, color: Charte.primaire),
                  const SizedBox(width: 10),
                  Expanded(
                    child: Text(
                      objectif!.phrase,
                      style: const TextStyle(
                          fontSize: 13, fontWeight: FontWeight.w600),
                    ),
                  ),
                ],
              ),
            ),
          ] else if (moi.rang == 1) ...[
            const SizedBox(height: 14),
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                const Icon(Icons.workspace_premium,
                    size: 18, color: Color(0xFFE0B341)),
                const SizedBox(width: 8),
                Text(
                  'Première place de la section ce mois-ci',
                  style: TextStyle(
                      fontSize: 13,
                      fontWeight: FontWeight.w600,
                      color: context.couleurs.onSurface),
                ),
              ],
            ),
          ],
        ],
      ),
    );
  }
}

class _LigneClassement extends StatelessWidget {
  const _LigneClassement({required this.ligne, required this.moi});

  final RangEnseignant ligne;
  final bool moi;

  @override
  Widget build(BuildContext context) {
    return CarteApp(
      padding: const EdgeInsets.symmetric(horizontal: 13, vertical: 11),
      couleur: moi ? Charte.primaire.withOpacity(0.07) : null,
      child: Row(
        children: [
          SizedBox(
            width: 26,
            child: Text(
              '${ligne.rang}',
              style: TextStyle(
                fontSize: 15,
                fontWeight: FontWeight.w800,
                color: context.encreDouce,
              ),
            ),
          ),
          Container(
            width: 34,
            height: 34,
            decoration: BoxDecoration(
              color: Charte.primaire.withOpacity(0.12),
              shape: BoxShape.circle,
            ),
            child: Center(
              child: Text(
                ligne.initiale,
                style: const TextStyle(
                    fontSize: 14,
                    fontWeight: FontWeight.w700,
                    color: Charte.primaire),
              ),
            ),
          ),
          const SizedBox(width: 11),
          Expanded(
            child: Text(
              moi ? '${ligne.nom} (vous)' : ligne.nom,
              maxLines: 1,
              overflow: TextOverflow.ellipsis,
              style: TextStyle(
                fontSize: 14,
                fontWeight: moi ? FontWeight.w800 : FontWeight.w600,
              ),
            ),
          ),
          Column(
            crossAxisAlignment: CrossAxisAlignment.end,
            children: [
              Text('${ligne.joursComplets} j',
                  style: const TextStyle(
                      fontSize: 13.5, fontWeight: FontWeight.w700)),
              Text('${ligne.heures.toStringAsFixed(0)} h',
                  style:
                      TextStyle(fontSize: 11, color: context.encreDouce)),
            ],
          ),
        ],
      ),
    );
  }
}

/// La méthode de calcul est écrite noir sur blanc : un classement dont on
/// ignore la règle se conteste au lieu de motiver.
class _MentionMethode extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Row(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Icon(Icons.info_outline, size: 14, color: context.encreDouce),
        const SizedBox(width: 8),
        Expanded(
          child: Text(
            'Classement sur les journées complètes du mois (arrivée et départ '
            'pointés), les heures cumulées départageant les ex æquo. Seuls les '
            'enseignants ayant pointé au moins une fois apparaissent.',
            style: TextStyle(
                fontSize: 11.5, height: 1.4, color: context.encreDouce),
          ),
        ),
      ],
    );
  }
}
