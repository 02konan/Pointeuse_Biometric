import 'package:flutter/material.dart';

/// Bloc affiché quand une liste est vide.
class EtatVide extends StatelessWidget {
  const EtatVide({super.key, required this.icone, required this.message});

  final IconData icone;
  final String message;

  @override
  Widget build(BuildContext context) {
    final couleur = Theme.of(context).colorScheme.outline;
    return Center(
      child: Padding(
        padding: const EdgeInsets.all(32),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Icon(icone, size: 46, color: couleur),
            const SizedBox(height: 12),
            Text(message, textAlign: TextAlign.center,
                style: TextStyle(color: couleur)),
          ],
        ),
      ),
    );
  }
}

/// Bloc d'erreur avec bouton de réessai.
class EtatErreur extends StatelessWidget {
  const EtatErreur({super.key, required this.message, this.onReessayer});

  final String message;
  final VoidCallback? onReessayer;

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Padding(
        padding: const EdgeInsets.all(32),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Icon(Icons.cloud_off,
                size: 46, color: Theme.of(context).colorScheme.error),
            const SizedBox(height: 12),
            Text(message, textAlign: TextAlign.center),
            if (onReessayer != null) ...[
              const SizedBox(height: 16),
              OutlinedButton.icon(
                onPressed: onReessayer,
                icon: const Icon(Icons.refresh),
                label: const Text('Réessayer'),
              ),
            ],
          ],
        ),
      ),
    );
  }
}

/// Tuile de statistique (libellé, valeur, icône colorée).
class CarteStat extends StatelessWidget {
  const CarteStat({
    super.key,
    required this.libelle,
    required this.valeur,
    required this.icone,
    required this.couleur,
  });

  final String libelle;
  final String valeur;
  final IconData icone;
  final Color couleur;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(14),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Container(
              padding: const EdgeInsets.all(8),
              decoration: BoxDecoration(
                color: couleur.withOpacity(0.12),
                borderRadius: BorderRadius.circular(10),
              ),
              child: Icon(icone, color: couleur, size: 20),
            ),
            const SizedBox(height: 10),
            Text(valeur,
                style: Theme.of(context)
                    .textTheme
                    .headlineSmall
                    ?.copyWith(fontWeight: FontWeight.bold)),
            Text(libelle,
                style: Theme.of(context).textTheme.bodySmall,
                maxLines: 2,
                overflow: TextOverflow.ellipsis),
          ],
        ),
      ),
    );
  }
}
