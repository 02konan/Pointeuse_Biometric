import 'package:flutter/material.dart';

/// Charte visuelle de l'application.
///
/// Les couleurs de statut (présent / incomplet / absent) sont **réservées** :
/// elles ne servent jamais à distinguer des séries dans un graphique, et ne
/// portent jamais une information seules. Vérification faite : en protanopie,
/// le vert et l'orange ne se séparent qu'à ΔE 7, et l'orange manque de
/// contraste sur fond clair — d'où la règle appliquée partout dans l'app :
/// **pastille colorée + icône + libellé**, le texte restant en encre neutre.
class Charte {
  // Teinte de marque
  static const Color primaire = Color(0xFF4361EE);
  static const Color primaireSombre = Color(0xFF2F3FB8);
  static const Color violet = Color(0xFF7B3FE4);

  // Statuts — clair
  static const Color succes = Color(0xFF16A34A);
  static const Color alerte = Color(0xFFF59E0B);
  static const Color danger = Color(0xFFDC2626);

  // Statuts — sombre (mêmes rôles, relevés pour tenir sur fond foncé)
  static const Color succesSombre = Color(0xFF4ADE80);
  static const Color alerteSombre = Color(0xFFFBBF24);
  static const Color dangerSombre = Color(0xFFF87171);

  /// Dégradé de l'en-tête : la signature visuelle de l'application.
  static const LinearGradient enTete = LinearGradient(
    begin: Alignment.topLeft,
    end: Alignment.bottomRight,
    colors: [Color(0xFF3A3FA8), primaire, Color(0xFF6D4BE8)],
  );

  static ThemeData clair() => _base(Brightness.light);
  static ThemeData sombre() => _base(Brightness.dark);

  static ThemeData _base(Brightness luminosite) {
    final sombre = luminosite == Brightness.dark;
    final schema = ColorScheme.fromSeed(
      seedColor: primaire,
      brightness: luminosite,
    );
    return ThemeData(
      useMaterial3: true,
      colorScheme: schema,
      scaffoldBackgroundColor:
          sombre ? const Color(0xFF101320) : const Color(0xFFF4F6FB),
      appBarTheme: AppBarTheme(
        centerTitle: false,
        elevation: 0,
        backgroundColor: schema.surface,
        foregroundColor: schema.onSurface,
      ),
      inputDecorationTheme: InputDecorationTheme(
        filled: true,
        border: OutlineInputBorder(
          borderRadius: BorderRadius.circular(12),
          borderSide: BorderSide(color: schema.outlineVariant),
        ),
      ),
      filledButtonTheme: FilledButtonThemeData(
        style: FilledButton.styleFrom(
          minimumSize: const Size.fromHeight(50),
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
          textStyle: const TextStyle(fontWeight: FontWeight.w600, fontSize: 15),
        ),
      ),
    );
  }

  /// Couleur de statut adaptée au thème courant.
  static Color statut(BuildContext context, String etat) {
    final sombre = Theme.of(context).brightness == Brightness.dark;
    return switch (etat) {
      'succes' => sombre ? succesSombre : succes,
      'alerte' => sombre ? alerteSombre : alerte,
      'danger' => sombre ? dangerSombre : danger,
      _ => primaire,
    };
  }
}

/// Raccourcis de surface, pour éviter de répéter Theme.of(context) partout.
extension SurfacesTheme on BuildContext {
  ColorScheme get couleurs => Theme.of(this).colorScheme;
  TextTheme get textes => Theme.of(this).textTheme;
  bool get estSombre => Theme.of(this).brightness == Brightness.dark;

  /// Fond des cartes : légèrement décollé du fond de page.
  Color get surfaceCarte => estSombre ? const Color(0xFF181D2B) : Colors.white;
  Color get filet => estSombre ? const Color(0xFF29304A) : const Color(0xFFE3E6EF);
  Color get encreDouce => estSombre ? const Color(0xFF9AA3BD) : const Color(0xFF6B7280);
}

/// Carte de base : une seule définition du rayon, du filet et de l'ombre,
/// pour que toutes les cartes de l'application aient exactement les mêmes bords.
class CarteApp extends StatelessWidget {
  const CarteApp({super.key, required this.enfant, this.padding, this.couleur});

  final Widget enfant;
  final EdgeInsetsGeometry? padding;
  final Color? couleur;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: padding ?? const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: couleur ?? context.surfaceCarte,
        borderRadius: BorderRadius.circular(16),
        border: Border.all(color: context.filet),
      ),
      child: enfant,
    );
  }
}

/// Étiquette de section : petites capitales espacées, en encre discrète.
class TitreSection extends StatelessWidget {
  const TitreSection(this.texte, {super.key, this.action});

  final String texte;
  final Widget? action;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.fromLTRB(2, 24, 2, 12),
      child: Row(
        children: [
          Expanded(
            child: Text(
              texte.toUpperCase(),
              style: TextStyle(
                fontSize: 11,
                fontWeight: FontWeight.w700,
                letterSpacing: 1.1,
                color: context.encreDouce,
              ),
            ),
          ),
          if (action != null) action!,
        ],
      ),
    );
  }
}
