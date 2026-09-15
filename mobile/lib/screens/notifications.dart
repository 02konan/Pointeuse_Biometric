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

            final nonLues = notifications.where((n) => !n.lue).length;

            return ListView(
              padding: const EdgeInsets.fromLTRB(16, 8, 16, 28),
              children: [
                if (nonLues > 0)
                  Padding(
                    padding: const EdgeInsets.only(bottom: 14),
                    child: Row(
                      children: [
                        Container(
                          width: 7,
                          height: 7,
                          decoration: const BoxDecoration(
                            color: Charte.primaire,
                            shape: BoxShape.circle,
                          ),
                        ),
                        const SizedBox(width: 8),
                        Text(
                          nonLues > 1
                              ? '$nonLues notifications non lues'
                              : '1 notification non lue',
                          style: TextStyle(
                            fontSize: 12.5,
                            fontWeight: FontWeight.w600,
                            color: context.encreDouce,
                          ),
                        ),
                      ],
                    ),
                  ),
                for (var i = 0; i < notifications.length; i++) ...[
                  _CarteNotification(
                    notification: notifications[i],
                    onLue: () => _marquerLue(notifications[i]),
                  ),
                  if (i < notifications.length - 1) const SizedBox(height: 10),
                ],
              ],
            );
          },
        ),
      ),
    );
  }
}

/// Une notification : le type porte une icône dédiée, jamais une couleur seule.
class _CarteNotification extends StatelessWidget {
  const _CarteNotification({required this.notification, required this.onLue});

  final modeles.Notification notification;
  final VoidCallback onLue;

  @override
  Widget build(BuildContext context) {
    final (icone, etat) = switch (notification.type) {
      'pointage' => (Icons.fingerprint, 'neutre'),
      'retard' => (Icons.schedule, 'alerte'),
      'absence' => (Icons.person_off_outlined, 'danger'),
      _ => (Icons.info_outline, 'neutre'),
    };
    final couleur = Charte.statut(context, etat);
    final lue = notification.lue;

    return InkWell(
      onTap: lue ? null : onLue,
      borderRadius: BorderRadius.circular(16),
      child: Container(
        padding: const EdgeInsets.all(14),
        decoration: BoxDecoration(
          color: context.surfaceCarte,
          borderRadius: BorderRadius.circular(16),
          border: Border.all(
            color: lue ? context.filet : Charte.primaire.withOpacity(0.45),
          ),
        ),
        child: Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Container(
              width: 36,
              height: 36,
              decoration: BoxDecoration(
                color: couleur.withOpacity(0.13),
                borderRadius: BorderRadius.circular(11),
              ),
              child: Icon(icone, color: couleur, size: 19),
            ),
            const SizedBox(width: 12),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Row(
                    children: [
                      Expanded(
                        child: Text(
                          notification.titre,
                          style: TextStyle(
                            fontSize: 14,
                            fontWeight:
                                lue ? FontWeight.w500 : FontWeight.w700,
                          ),
                        ),
                      ),
                      if (!lue)
                        Container(
                          width: 8,
                          height: 8,
                          decoration: const BoxDecoration(
                            color: Charte.primaire,
                            shape: BoxShape.circle,
                          ),
                        ),
                    ],
                  ),
                  const SizedBox(height: 3),
                  Text(
                    notification.message,
                    style: TextStyle(
                      fontSize: 12.5,
                      height: 1.4,
                      color: context.encreDouce,
                    ),
                  ),
                  const SizedBox(height: 6),
                  Text(
                    notification.date,
                    style: TextStyle(fontSize: 11, color: context.encreDouce),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}
