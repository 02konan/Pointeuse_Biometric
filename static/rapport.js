/* =====================================================================
   BiometricWeb — page Rapports
   Génération des fiches PDF (admin / superadmin), liste des rapports
   déjà produits, téléchargement, impression et suppression.
   ===================================================================== */
(function () {
  "use strict";

  const corps = () => document.getElementById("table-rapports");

  function notifier(message, type) {
    if (typeof window.bwToast === "function") window.bwToast(message, type);
    else alert(message);
  }

  function ligneRapport(data) {
    const tr = document.createElement("tr");
    const nom = data.nom || "";
    tr.innerHTML = `
      <td>
        <div class="d-flex align-items-center gap-2">
          <i class="fas fa-file-pdf text-danger"></i>
          <span class="fw-semibold text-break">${nom}</span>
        </div>
      </td>
      <td><span class="badge bg-primary-soft text-primary">${data.type || "Présence"}</span></td>
      <td>${data.auteur || "—"}</td>
      <td class="text-muted">${data.date || "—"}</td>
      <td class="text-end text-nowrap">
        <a href="/telechargement/${encodeURIComponent(nom)}" class="btn btn-sm btn-outline-primary me-1" title="Télécharger">
          <i class="fas fa-download"></i>
        </a>
        <a href="/impression/${encodeURIComponent(nom)}" target="_blank" rel="noopener"
           class="btn btn-sm btn-outline-secondary me-1" title="Ouvrir / imprimer">
          <i class="fas fa-print"></i>
        </a>
        <button type="button" class="btn btn-sm btn-outline-danger" data-supprimer="${encodeURIComponent(nom)}" title="Supprimer">
          <i class="fas fa-trash"></i>
        </button>
      </td>`;
    return tr;
  }

  function ajouterLigne(data, enTete) {
    const tbody = corps();
    if (!tbody) return;
    const tr = ligneRapport(data);
    const vide = tbody.querySelector('tr[data-empty-state="true"]');
    if (enTete && vide && vide.nextSibling) tbody.insertBefore(tr, vide.nextSibling);
    else if (enTete) tbody.appendChild(tr);
    else tbody.appendChild(tr);
    rafraichirPagination();
  }

  function rafraichirPagination() {
    if (window.bwPagination) window.bwPagination.rafraichir();
  }

  function chargerListe() {
    const tbody = corps();
    if (!tbody) return;
    fetch("/api/liste_rapports")
      .then((r) => r.json())
      .then((liste) => {
        tbody.querySelectorAll('tr:not([data-empty-state="true"])').forEach((tr) => tr.remove());
        (liste || []).forEach((item) => ajouterLigne(item, false));
        rafraichirPagination();
      })
      .catch(() => notifier("Impossible de charger la liste des rapports.", "danger"));
  }

  function supprimer(nomEncode, bouton) {
    if (!confirm("Voulez-vous vraiment supprimer ce rapport ?")) return;
    fetch("/suppression/" + nomEncode, { method: "DELETE" })
      .then((r) => r.json())
      .then((data) => {
        if (data.success) {
          const tr = bouton.closest("tr");
          if (tr) tr.remove();
          rafraichirPagination();
          notifier("Rapport supprimé.", "success");
        } else {
          notifier(data.error || "Suppression impossible.", "danger");
        }
      })
      .catch(() => notifier("Erreur réseau lors de la suppression.", "danger"));
  }

  function valeur(id) {
    const el = document.getElementById(id);
    return el ? el.value : "";
  }

  // Génère un rapport et gère l'état visuel du bouton.
  function generer(bouton, url, charge) {
    const libelleInitial = bouton.innerHTML;
    bouton.innerHTML = '<i class="fas fa-spinner fa-spin me-2"></i>Génération en cours…';
    bouton.disabled = true;

    fetch(url, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(charge),
    })
      .then(async (reponse) => {
        const resultat = await reponse.json().catch(() => ({}));
        if (!reponse.ok || !resultat.success) {
          throw new Error(resultat.error || "Erreur lors de la génération du rapport.");
        }
        ajouterLigne(resultat, true);
        notifier("Rapport généré : " + resultat.nom, "success");
      })
      .catch((e) => notifier(e.message, "danger"))
      .finally(() => {
        bouton.innerHTML = libelleInitial;
        bouton.disabled = false;
      });
  }

  document.addEventListener("DOMContentLoaded", function () {
    if (!corps()) return;

    chargerListe();

    const rafraichir = document.getElementById("btn-rafraichir-rapports");
    if (rafraichir) rafraichir.addEventListener("click", chargerListe);

    corps().addEventListener("click", function (e) {
      const bouton = e.target.closest("[data-supprimer]");
      if (bouton) supprimer(bouton.getAttribute("data-supprimer"), bouton);
    });

    // --- Formulaire admin (section de l'utilisateur connecté) ---
    const btnAdmin = document.getElementById("btn-fiche-presence");
    if (btnAdmin) {
      btnAdmin.addEventListener("click", function () {
        const debut = valeur("date_debut");
        const fin = valeur("date_fin");
        if (!debut || !fin) {
          notifier("Veuillez sélectionner une date de début et une date de fin.", "warning");
          return;
        }
        if (debut > fin) {
          notifier("La date de début doit précéder la date de fin.", "warning");
          return;
        }
        generer(btnAdmin, "/api/fiche_presence", {
          date_debut: debut,
          date_fin: fin,
          idEmploye: valeur("idEmploye"),
        });
      });
    }

    // --- Formulaire superadmin (toutes sections) ---
    const btnSuper = document.getElementById("btn-fiche-presence-admin");
    if (btnSuper) {
      btnSuper.addEventListener("click", function () {
        const debut = valeur("date_debut_admin");
        const fin = valeur("date_fin_admin");
        if (!debut || !fin) {
          notifier("Veuillez sélectionner une date de début et une date de fin.", "warning");
          return;
        }
        if (debut > fin) {
          notifier("La date de début doit précéder la date de fin.", "warning");
          return;
        }
        generer(btnSuper, "/api/fiche_presence_admin", {
          date_admin_debut: debut,
          date_admin_fin: fin,
          employeeid_admin: valeur("idEmploye_admin"),
          section_admin: valeur("sectionid"),
        });
      });
    }
  });
})();
