/// Modèles de données renvoyés par l'API Flask.
///
/// Chaque classe sait se construire depuis le JSON de l'API ; les champs
/// absents ou nuls sont tolérés, l'application ne doit jamais planter sur
/// une donnée manquante.

class Utilisateur {
  final String nom;
  final String role;
  final String? matricule;
  final String type;

  const Utilisateur({
    required this.nom,
    required this.role,
    required this.type,
    this.matricule,
  });

  bool get estEnseignant => matricule != null && matricule!.isNotEmpty;

  factory Utilisateur.depuisJson(Map<String, dynamic> json) => Utilisateur(
        nom: json['nom']?.toString() ?? '',
        role: json['role']?.toString() ?? '',
        type: json['type']?.toString() ?? '',
        matricule: json['matricule']?.toString(),
      );
}

class Profil {
  final String matricule;
  final String nom;
  final String? telephone;
  final String? email;
  final String? adresse;
  final String? poste;
  final String? section;
  final String? dateEmbauche;

  const Profil({
    required this.matricule,
    required this.nom,
    this.telephone,
    this.email,
    this.adresse,
    this.poste,
    this.section,
    this.dateEmbauche,
  });

  factory Profil.depuisJson(Map<String, dynamic> json) => Profil(
        matricule: json['matricule']?.toString() ?? '',
        nom: json['nom']?.toString() ?? '',
        telephone: json['telephone']?.toString(),
        email: json['email']?.toString(),
        adresse: json['adresse']?.toString(),
        poste: json['poste']?.toString(),
        section: json['section']?.toString(),
        dateEmbauche: json['date_embauche']?.toString(),
      );
}

class Pointage {
  final String date;
  final String? entree;
  final String? sortie;
  final String? duree;
  final int nbPassages;
  final bool complet;
  final String? statut;

  const Pointage({
    required this.date,
    required this.nbPassages,
    required this.complet,
    this.entree,
    this.sortie,
    this.duree,
    this.statut,
  });

  factory Pointage.depuisJson(Map<String, dynamic> json) => Pointage(
        date: json['date']?.toString() ?? '',
        entree: json['entree']?.toString(),
        sortie: json['sortie']?.toString(),
        duree: json['duree']?.toString(),
        nbPassages: (json['nb_passages'] as num?)?.toInt() ?? 0,
        complet: json['complet'] == true,
        statut: json['statut']?.toString(),
      );

  /// Heure seule, sans les secondes (« 07:58:00 » → « 07:58 »).
  static String heure(String? valeur) {
    if (valeur == null || valeur.isEmpty) return '--:--';
    final parties = valeur.split(':');
    if (parties.length < 2) return valeur;
    return '${parties[0]}:${parties[1]}';
  }
}

class Statistiques {
  final String mois;
  final int joursPointes;
  final int joursComplets;
  final int joursIncomplets;
  final double heuresCumulees;
  final double dureeMoyenne;

  const Statistiques({
    required this.mois,
    required this.joursPointes,
    required this.joursComplets,
    required this.joursIncomplets,
    required this.heuresCumulees,
    required this.dureeMoyenne,
  });

  factory Statistiques.depuisJson(Map<String, dynamic> json) => Statistiques(
        mois: json['mois']?.toString() ?? '',
        joursPointes: (json['jours_pointes'] as num?)?.toInt() ?? 0,
        joursComplets: (json['jours_complets'] as num?)?.toInt() ?? 0,
        joursIncomplets: (json['jours_incomplets'] as num?)?.toInt() ?? 0,
        heuresCumulees: (json['heures_cumulees'] as num?)?.toDouble() ?? 0,
        dureeMoyenne: (json['duree_moyenne_heures'] as num?)?.toDouble() ?? 0,
      );
}

class Cours {
  final String jour;
  final String? matiere;
  final String? heureArrivee;
  final String? heureDepart;
  final String? duree;
  final String? section;
  final String? type;

  const Cours({
    required this.jour,
    this.matiere,
    this.heureArrivee,
    this.heureDepart,
    this.duree,
    this.section,
    this.type,
  });

  factory Cours.depuisJson(Map<String, dynamic> json) => Cours(
        jour: json['jour']?.toString() ?? '',
        matiere: json['matiere']?.toString(),
        heureArrivee: json['heure_arrivee']?.toString(),
        heureDepart: json['heure_depart']?.toString(),
        duree: json['duree']?.toString(),
        section: json['section']?.toString(),
        type: json['type']?.toString(),
      );
}

class Notification {
  final int id;
  final String titre;
  final String message;
  final String type;
  final bool lue;
  final String date;

  const Notification({
    required this.id,
    required this.titre,
    required this.message,
    required this.type,
    required this.lue,
    required this.date,
  });

  factory Notification.depuisJson(Map<String, dynamic> json) => Notification(
        id: (json['id'] as num?)?.toInt() ?? 0,
        titre: json['titre']?.toString() ?? '',
        message: json['message']?.toString() ?? '',
        type: json['type']?.toString() ?? 'info',
        lue: json['lue'] == true,
        date: json['date']?.toString() ?? '',
      );
}
