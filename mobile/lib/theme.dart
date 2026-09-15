import 'package:flutter/material.dart';

/// Charte visuelle alignée sur l'interface web (bleu #4361EE, fond clair).
class Charte {
  static const Color primaire = Color(0xFF4361EE);
  static const Color succes = Color(0xFF16A34A);
  static const Color danger = Color(0xFFDC2626);
  static const Color alerte = Color(0xFFF59E0B);

  static ThemeData clair() => _base(Brightness.light);
  static ThemeData sombre() => _base(Brightness.dark);

  static ThemeData _base(Brightness luminosite) {
    final schema = ColorScheme.fromSeed(
      seedColor: primaire,
      brightness: luminosite,
    );
    return ThemeData(
      useMaterial3: true,
      colorScheme: schema,
      scaffoldBackgroundColor:
          luminosite == Brightness.light ? const Color(0xFFF4F6FB) : null,
      appBarTheme: AppBarTheme(
        centerTitle: false,
        elevation: 0,
        backgroundColor: schema.surface,
        foregroundColor: schema.onSurface,
      ),
      inputDecorationTheme: InputDecorationTheme(
        filled: true,
        border: OutlineInputBorder(
          borderRadius: BorderRadius.circular(10),
          borderSide: BorderSide(color: schema.outlineVariant),
        ),
      ),
      filledButtonTheme: FilledButtonThemeData(
        style: FilledButton.styleFrom(
          minimumSize: const Size.fromHeight(48),
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(10)),
        ),
      ),
    );
  }
}
