import 'dart:math' as math;

import 'package:flutter/material.dart';

import '../theme.dart';

/// Anneau de progression portant un chiffre unique au centre.
///
/// Sert au taux d'assiduité du mois : une seule valeur, donc pas un graphique
/// à séries mais un chiffre mis en scène. Le pourcentage est écrit en toutes
/// lettres au centre — l'arc n'est qu'un renfort visuel, jamais le seul
/// porteur de l'information.
class AnneauTaux extends StatelessWidget {
  const AnneauTaux({
    super.key,
    required this.taux,
    required this.libelle,
    this.taille = 132,
  });

  /// Valeur entre 0 et 1.
  final double taux;
  final String libelle;
  final double taille;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: taille,
      height: taille,
      child: CustomPaint(
        painter: _PeintreAnneau(
          taux: taux.clamp(0, 1),
          piste: context.filet,
          trait: Charte.primaire,
        ),
        child: Center(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              Text(
                '${(taux * 100).round()}',
                style: TextStyle(
                  fontSize: taille * 0.27,
                  fontWeight: FontWeight.w800,
                  height: 1,
                  letterSpacing: -1,
                  color: context.couleurs.onSurface,
                ),
              ),
              const SizedBox(height: 2),
              Text(
                libelle,
                textAlign: TextAlign.center,
                style: TextStyle(
                  fontSize: 10,
                  fontWeight: FontWeight.w600,
                  letterSpacing: 0.6,
                  color: context.encreDouce,
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}

class _PeintreAnneau extends CustomPainter {
  _PeintreAnneau({required this.taux, required this.piste, required this.trait});

  final double taux;
  final Color piste;
  final Color trait;

  @override
  void paint(Canvas canvas, Size size) {
    const epaisseur = 11.0;
    final rect = Rect.fromLTWH(
      epaisseur / 2,
      epaisseur / 2,
      size.width - epaisseur,
      size.height - epaisseur,
    );

    final fond = Paint()
      ..color = piste
      ..style = PaintingStyle.stroke
      ..strokeWidth = epaisseur
      ..strokeCap = StrokeCap.round;
    canvas.drawArc(rect, -math.pi / 2, math.pi * 2, false, fond);

    if (taux <= 0) return;

    final valeur = Paint()
      ..color = trait
      ..style = PaintingStyle.stroke
      ..strokeWidth = epaisseur
      ..strokeCap = StrokeCap.round;
    canvas.drawArc(rect, -math.pi / 2, math.pi * 2 * taux, false, valeur);
  }

  @override
  bool shouldRepaint(_PeintreAnneau ancien) =>
      ancien.taux != taux || ancien.trait != trait || ancien.piste != piste;
}

/// Une barre du graphique hebdomadaire.
class BarreJour {
  const BarreJour({
    required this.libelle,
    required this.heures,
    required this.complet,
  });

  final String libelle;
  final double heures;
  final bool complet;
}

/// Heures travaillées par jour.
///
/// Une seule série, donc une seule teinte et pas de légende : le titre de la
/// carte nomme la mesure. Seule la plus haute barre porte son chiffre, pour
/// éviter d'écrire une valeur sur chaque colonne. Les journées sans départ
/// enregistré sont hachurées et rappelées sous le graphique — leur durée est
/// inconnue, pas nulle.
class BarresHeures extends StatelessWidget {
  const BarresHeures({super.key, required this.jours, this.hauteur = 150});

  final List<BarreJour> jours;
  final double hauteur;

  @override
  Widget build(BuildContext context) {
    if (jours.isEmpty) {
      return SizedBox(
        height: hauteur,
        child: Center(
          child: Text(
            'Aucun pointage sur la période',
            style: TextStyle(color: context.encreDouce, fontSize: 12),
          ),
        ),
      );
    }

    return SizedBox(
      height: hauteur,
      width: double.infinity,
      child: CustomPaint(
        painter: _PeintreBarres(
          jours: jours,
          barre: Charte.primaire,
          grille: context.filet,
          encre: context.couleurs.onSurface,
          encreDouce: context.encreDouce,
        ),
      ),
    );
  }
}

class _PeintreBarres extends CustomPainter {
  _PeintreBarres({
    required this.jours,
    required this.barre,
    required this.grille,
    required this.encre,
    required this.encreDouce,
  });

  final List<BarreJour> jours;
  final Color barre;
  final Color grille;
  final Color encre;
  final Color encreDouce;

  static const double _hauteurAxe = 20;
  static const double _hauteurLabel = 16;

  @override
  void paint(Canvas canvas, Size size) {
    final maxi = jours.fold<double>(0, (m, j) => math.max(m, j.heures));
    // Échelle arrondie à l'heure supérieure : les graduations tombent juste.
    final plafond = maxi <= 0 ? 4.0 : maxi.ceilToDouble();
    final hautTrace = _hauteurLabel;
    final basTrace = size.height - _hauteurAxe;
    final hauteurTrace = basTrace - hautTrace;

    // Grille discrète : trois niveaux, étiquetés.
    final traitGrille = Paint()
      ..color = grille
      ..strokeWidth = 1;
    for (var i = 0; i <= 2; i++) {
      final valeur = plafond * i / 2;
      final y = basTrace - hauteurTrace * (i / 2);
      canvas.drawLine(Offset(26, y), Offset(size.width, y), traitGrille);
      _texte(
        canvas,
        valeur == valeur.roundToDouble()
            ? '${valeur.round()}h'
            : '${valeur.toStringAsFixed(1)}h',
        Offset(0, y - 6),
        10,
        encreDouce,
      );
    }

    final zone = size.width - 26;
    final pas = zone / jours.length;
    // 2px de fond entre deux barres voisines.
    final largeur = math.min(pas - 10, 26.0);

    final indexMax = jours.indexWhere((j) => j.heures == maxi && maxi > 0);

    for (var i = 0; i < jours.length; i++) {
      final jour = jours[i];
      final centre = 26 + pas * i + pas / 2;
      final hauteurBarre =
          plafond <= 0 ? 0.0 : hauteurTrace * (jour.heures / plafond);

      // Libellé du jour, toujours présent.
      _texte(
        canvas,
        jour.libelle,
        Offset(centre - 10, basTrace + 5),
        10,
        encreDouce,
        largeurMax: 20,
        centre: true,
      );

      if (jour.heures <= 0) {
        // Journée sans durée connue : un tiret, pas une barre à zéro.
        canvas.drawLine(
          Offset(centre - largeur / 3, basTrace - 1),
          Offset(centre + largeur / 3, basTrace - 1),
          Paint()
            ..color = encreDouce.withOpacity(0.5)
            ..strokeWidth = 2
            ..strokeCap = StrokeCap.round,
        );
        continue;
      }

      final rect = RRect.fromRectAndCorners(
        Rect.fromLTWH(
          centre - largeur / 2,
          basTrace - hauteurBarre,
          largeur,
          hauteurBarre,
        ),
        topLeft: const Radius.circular(4),
        topRight: const Radius.circular(4),
      );

      final peinture = Paint()..color = jour.complet ? barre : barre.withOpacity(0.35);
      canvas.drawRRect(rect, peinture);

      if (!jour.complet) {
        // Hachures : la journée est incomplète, la durée n'est qu'indicative.
        canvas.save();
        canvas.clipRRect(rect);
        final hachure = Paint()
          ..color = barre.withOpacity(0.55)
          ..strokeWidth = 2;
        for (var x = -hauteurBarre; x < largeur + hauteurBarre; x += 6) {
          canvas.drawLine(
            Offset(centre - largeur / 2 + x, basTrace),
            Offset(centre - largeur / 2 + x + hauteurBarre, basTrace - hauteurBarre),
            hachure,
          );
        }
        canvas.restore();
      }

      // Étiquette directe sur la seule barre la plus haute.
      if (i == indexMax) {
        _texte(
          canvas,
          '${jour.heures.toStringAsFixed(1)}h',
          Offset(centre - 16, basTrace - hauteurBarre - 15),
          10.5,
          encre,
          largeurMax: 32,
          centre: true,
          gras: true,
        );
      }
    }
  }

  void _texte(
    Canvas canvas,
    String valeur,
    Offset position,
    double taille,
    Color couleur, {
    double largeurMax = 30,
    bool centre = false,
    bool gras = false,
  }) {
    final peintre = TextPainter(
      text: TextSpan(
        text: valeur,
        style: TextStyle(
          fontSize: taille,
          color: couleur,
          fontWeight: gras ? FontWeight.w700 : FontWeight.w500,
        ),
      ),
      textDirection: TextDirection.ltr,
      textAlign: centre ? TextAlign.center : TextAlign.left,
    )..layout(maxWidth: largeurMax);
    peintre.paint(canvas, position);
  }

  @override
  bool shouldRepaint(_PeintreBarres ancien) => true;
}

/// Pastille de statut : couleur **et** icône **et** libellé.
///
/// Jamais la couleur seule : vert et orange se confondent en protanopie.
class PastilleStatut extends StatelessWidget {
  const PastilleStatut({
    super.key,
    required this.etat,
    required this.libelle,
    required this.icone,
  });

  final String etat;
  final String libelle;
  final IconData icone;

  @override
  Widget build(BuildContext context) {
    final couleur = Charte.statut(context, etat);
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 5),
      decoration: BoxDecoration(
        color: couleur.withOpacity(0.13),
        borderRadius: BorderRadius.circular(999),
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Icon(icone, size: 13, color: couleur),
          const SizedBox(width: 5),
          Text(
            libelle,
            style: TextStyle(
              fontSize: 11.5,
              fontWeight: FontWeight.w600,
              // Le texte reste en encre, pas en couleur de statut.
              color: context.couleurs.onSurface,
            ),
          ),
        ],
      ),
    );
  }
}
