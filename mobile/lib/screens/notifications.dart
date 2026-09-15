import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../models/modeles.dart' as modeles;
import '../services/api.dart';
import '../services/session.dart';
import '../theme.dart';
import '../widgets/communs.dart';

/// Liste des notifications, marquées comme lues à l'ouverture.
class NotificationsEcran extends StatefulWidget {
  const NotificationsEcran({super.key});

  @override
  State<NotificationsEcran> createState() => _NotificationsEcranState();
}

class _NotificationsEcranState extends State<NotificationsEcran> {
  late Future<List<modeles.Notification>> _notifications;

  @override
  void initState() {
    super.initState();
    _notifications = context.read<Session>().api.notifications();
  }

  void _recharger() {
    setState(() => _notifications = context.read<Session>().api.notifications());
  }

  Future<void> _marquerLue(modeles.Notification n) async {
    if (n.lue) return;
    try {
      await context.read<Session>().api.marquerLue(n.id);
      _recharger();
    } on ErreurApi catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context)
          .showSnackBar(SnackBar(content: Text(e.message)));
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Notifications')),
      body: RefreshIndicator(
        onRefresh: () async => _recharger(),
        child: FutureBuilder<List<modeles.Notification>>(
          future: _notifications,
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
                  onReessayer: _recharger,
                ),
              ]);
            }

            final notifications = snapshot.data ?? [];
            if (notifications.isEmpty) {
              return ListView(children: const [
                SizedBox(height: 60),
                EtatVide(
                    icone: Icons.notifications_none,
                    message: 'Aucune notification'),
              ]);
            }

            return ListView.separated(
              padding: const EdgeInsets.all(16),
              itemCount: notifications.length,
              separatorBuilder: (_, __) => const SizedBox(height: 8),
              itemBuilder: (context, i) {
                final n = notifications[i];
                final (icone, couleur) = switch (n.type) {
                  'pointage' => (Icons.fingerprint, Charte.primaire),
                  'retard' => (Icons.schedule, Charte.alerte),
                  'absence' => (Icons.person_off_outlined, Charte.danger),
                  _ => (Icons.info_outline, Charte.primaire),
                };
                return Card(
                  color: n.lue
                      ? null
                      : Charte.primaire.withOpacity(0.06),
                  child: ListTile(
                    onTap: () => _marquerLue(n),
                    leading: CircleAvatar(
                      backgroundColor: couleur.withOpacity(0.12),
                      child: Icon(icone, color: couleur, size: 20),
                    ),
                    title: Text(
                      n.titre,
                      style: TextStyle(
                          fontWeight:
                              n.lue ? FontWeight.normal : FontWeight.bold),
                    ),
                    subtitle: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(n.message),
                        const SizedBox(height: 4),
                        Text(n.date,
                            style: Theme.of(context).textTheme.bodySmall),
                      ],
                    ),
                    trailing: n.lue
                        ? null
                        : const Icon(Icons.circle, size: 10, color: Charte.primaire),
                  ),
                );
              },
            );
          },
        ),
      ),
    );
  }
}
