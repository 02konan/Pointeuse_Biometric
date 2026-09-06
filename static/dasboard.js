/* =====================================================================
   BiometricWeb — tableau de bord (admin de section / professeur)
   Alimente les compteurs, les barres de progression, les graphiques et
   la liste d'activité à partir de /api/dashboard.
   ===================================================================== */
(function () {
  "use strict";

  let attendanceChart = null;
  let attendancePieChart = null;
  let dernieresDonnees = null;

  const PALETTE = {
    primary: "#4361ee",
    danger: "#dc2626",
    warning: "#f59e0b",
  };

  function themeSombre() {
    return document.documentElement.getAttribute("data-bs-theme") === "dark";
  }

  function couleurGrille() {
    return themeSombre() ? "rgba(255,255,255,0.08)" : "rgba(15,23,42,0.07)";
  }

  function couleurTexte() {
    return themeSombre() ? "#94a1bb" : "#6b7689";
  }

  function optionsCommunes() {
    return {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: { labels: { color: couleurTexte(), usePointStyle: true, padding: 16 } },
      },
      scales: {
        x: { ticks: { color: couleurTexte() }, grid: { color: couleurGrille() } },
        y: {
          beginAtZero: true,
          ticks: { color: couleurTexte(), precision: 0 },
          grid: { color: couleurGrille() },
        },
      },
    };
  }

  function definir(id, valeur) {
    const el = document.getElementById(id);
    if (el) el.textContent = valeur ?? 0;
  }

  function barre(id, pourcentage) {
    const el = document.getElementById(id);
    if (!el) return;
    const p = Math.min(Math.max(Number(pourcentage) || 0, 0), 100);
    el.style.width = p + "%";
    el.setAttribute("aria-valuenow", p.toFixed(0));
  }

  function chargerDonnees() {
    fetch("/api/dashboard")
      .then((res) => res.json())
      .then((data) => {
        dernieresDonnees = data;

        afficherActivites(data["activité_recentes"] || data["activité_recentes_user"]);

        // Compteurs (les identifiants absents de la page sont ignorés)
        definir("presents-count", data.presents);
        definir("absents-count", data.absents);
        definir("retard-count", data.retard);
        definir("total-eleves", data.total_eleves);
        definir("presents-prof", data.presents_user);
        definir("absents-prof", data.absents_user);
        definir("retard-prof", data.retard_user);
        definir("new-pointage", data.pointage);

        barre("bar-present", data.pourcentage_presents ?? data.pourcentage_presents_user);
        barre("bar-absent", data.pourcentage_absents ?? data.pourcentage_absents_user);
        barre("bar-retard", data.pourcentage_retards ?? data.pourcentage_retards_user);
        barre("total-eleves-bar", data.total_eleves ? 100 : 0);

        if (data.total_eleves !== undefined) {
          const actifs = data.employes_actifs_mois ?? 0;
          const total = data.total_eleves || 0;
          const jours = data.jours_travailles_mois || 0;
          const maximum = total * jours;

          definir("presence-mois-count", actifs);
          definir("absence-mois-count", Math.max(total - actifs, 0));
          definir("retard-mois-count", data.employes_retard_mois);

          barre("bar-presence-mois", maximum ? (actifs / maximum) * 100 : 0);
          barre("bar-absence-mois", total ? ((total - actifs) / total) * 100 : 0);
          barre("bar-retard-mois", total ? ((data.employes_retard_mois ?? 0) / total) * 100 : 0);
        }

        dessinerGraphiques(data);
      })
      .catch((e) => console.error("Chargement du tableau de bord impossible :", e));
  }

  function afficherActivites(activites) {
    const conteneur = document.getElementById("recent-activity-list");
    if (!conteneur) return;

    if (!activites || !activites.length) {
      conteneur.innerHTML =
        '<li class="list-group-item"><div class="empty-state"><i class="fas fa-inbox"></i>Aucune activité récente</div></li>';
      return;
    }

    conteneur.innerHTML = "";
    activites.forEach(function (ligne) {
      const [nom, date, statut, section] = ligne;
      const arrivee = statut === "Arrivée enregistrée";
      const item = document.createElement("li");
      item.className = "list-group-item d-flex justify-content-between align-items-center gap-3";
      item.innerHTML = `
        <div class="d-flex align-items-center gap-2 min-w-0">
          <span class="stat-icon bg-${arrivee ? "success" : "danger"}-soft" style="width:38px;height:38px;font-size:.9rem">
            <i class="fas ${arrivee ? "fa-arrow-right-to-bracket" : "fa-arrow-right-from-bracket"} text-${arrivee ? "success" : "danger"}"></i>
          </span>
          <div class="min-w-0">
            <div class="fw-semibold text-truncate">${nom ?? ""}</div>
            <span class="badge bg-${arrivee ? "success" : "danger"}">${statut ?? ""}</span>
          </div>
        </div>
        <div class="text-end">
          <div class="small">${date ? new Date(date).toLocaleString("fr-FR") : ""}</div>
          ${section ? `<small class="text-muted">${section}</small>` : ""}
        </div>`;
      conteneur.appendChild(item);
    });
  }

  function dessinerGraphiques(data) {
    const prof = data.total_eleves === undefined;
    const presents = prof ? data.presents_user ?? 0 : data.presents ?? 0;
    const absents = prof ? data.absents_user ?? 0 : data.absents ?? 0;
    const retards = prof ? data.retard_user ?? 0 : data.retard ?? 0;

    const ligne = document.getElementById("attendanceChart");
    if (ligne) {
      if (attendanceChart) attendanceChart.destroy();
      attendanceChart = new Chart(ligne.getContext("2d"), {
        type: "bar",
        data: {
          labels: ["Présents", "Absents", "Retards"],
          datasets: [
            {
              label: "Statistiques du jour",
              data: [presents, absents, retards],
              backgroundColor: [PALETTE.primary, PALETTE.danger, PALETTE.warning],
              borderRadius: 8,
              maxBarThickness: 64,
            },
          ],
        },
        options: Object.assign(optionsCommunes(), {
          plugins: { legend: { display: false } },
        }),
      });
    }

    const camembert = document.getElementById("attendancePieChart");
    if (camembert) {
      if (attendancePieChart) attendancePieChart.destroy();
      attendancePieChart = new Chart(camembert.getContext("2d"), {
        type: "doughnut",
        data: {
          labels: ["Présents", "Absents", "Retards"],
          datasets: [
            {
              data: [presents, absents, retards],
              backgroundColor: [PALETTE.primary, PALETTE.danger, PALETTE.warning],
              borderWidth: 0,
            },
          ],
        },
        options: {
          responsive: true,
          maintainAspectRatio: false,
          cutout: "62%",
          plugins: {
            legend: {
              position: "bottom",
              labels: { color: couleurTexte(), usePointStyle: true, padding: 16 },
            },
          },
        },
      });
    }
  }

  document.addEventListener("DOMContentLoaded", function () {
    if (!document.getElementById("dashboard-content")) return;
    chargerDonnees();
    setInterval(chargerDonnees, 60000);
  });

  // Redessine les graphiques lorsque l'utilisateur change de thème.
  document.addEventListener("bw:theme", function () {
    if (dernieresDonnees) dessinerGraphiques(dernieresDonnees);
  });
})();
