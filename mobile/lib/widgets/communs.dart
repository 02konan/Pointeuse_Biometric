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
