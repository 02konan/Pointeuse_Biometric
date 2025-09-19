let attendanceChart = null;
let attendancePieChart = null;

function chargerDonneesDashboard() {
  fetch("/api/dashboard")
    .then(res => res.text()) // récupère d'abord le texte
    .then(text => {
      let data;
      try {
        data = JSON.parse(text);
      } catch (e) {
        console.error("Erreur JSON :", e, text);
        return;
      }

      // --- ACTIVITES RECENTES ---
      if (data["activité_recentes"]) {
        afficherActivites(data["activité_recentes"]);
      }

      // --- CHARTS ---
      afficherCharts(data);

      // --- COMPTEURS QUOTIDIENS ---
      const counters = [
        ["presents-count", data.presents],
        ["absents-count", data.absents],
        ["retard-count", data.retard],
        ["total-eleves", data.total_eleves],
        ["presents-prof", data.presents],
        ["absents-prof", data.absents],
        ["retard-prof", data.retard],
        ["new-pointage", data.pointage]
      ];

      counters.forEach(([id, value]) => {
        const el = document.getElementById(id);
        if (el) el.textContent = value ?? 0;
      });

      // --- BARRES DE PROGRESSION QUOTIDIENNES ---
      const bars = [
        ["bar-present", data.pourcentage_presents],
        ["bar-absent", data.pourcentage_absents],
        ["bar-retard", data.pourcentage_retards]
      ];

      bars.forEach(([id, pct]) => {
        const el = document.getElementById(id);
        if (el) el.style.width = (pct ?? 0) + "%";
      });

      // --- COMPTEURS MENSUELS ---
      const moisCounters = [
        ["presence-mois-count", data.employes_actifs_mois],
        ["absence-mois-count", data.total_eleves - (data.employes_actifs_mois ?? 0)],
        ["retard-mois-count", data.employes_retard_mois],
        ["new-employee-count", 0] // à 0 par défaut
      ];

      moisCounters.forEach(([id, value]) => {
        const el = document.getElementById(id);
        if (el) el.textContent = value ?? 0;
      });

      // --- BARRES DE PROGRESSION MENSUELLES ---
      const barsMois = [
        ["bar-presence-mois", data.pourcentage_actifs_mois],
        ["bar-absence-mois", data.pourcentage_inactifs_mois],
        ["bar-retard-mois", data.pourcentage_retards_mois],
        ["bar-new-employee", 0]
      ];

      barsMois.forEach(([id, pct]) => {
        const el = document.getElementById(id);
        if (el) el.style.width = (pct ?? 0) + "%";
      });

    })
    .catch(error => console.error("Erreur lors du chargement du dashboard :", error));
}

function afficherActivites(activites) {
  const container = document.getElementById("recent-activity-list");
  if (!container) return;
  container.innerHTML = "";

  activites.forEach(([id, date, Status]) => {
    const couleur = Status === "Arrivée enregistrée" ? "badge bg-success" : "badge bg-danger";

    const item = document.createElement("li");
    item.className = "list-group-item d-flex justify-content-between align-items-center p-3";
    item.innerHTML = `
      <div class="d-flex align-items-center">
         <div class="avatar me-2">
            <img src="static/images/icons8-life-cycle-96.png" alt="">
         </div>
        <div>
          <h6 class="mb-0">${id}</h6>
          <small class="${couleur}">${Status}</small>
        </div>
      </div>
      <span class="text-muted">${new Date(date).toLocaleString()}</span>
    `;
    container.appendChild(item);
  });
}

function afficherCharts(data) {
  const lineCanvas = document.getElementById("attendanceChart");
  const pieCanvas = document.getElementById("attendancePieChart");

  if (!lineCanvas || !pieCanvas) return;

  // Détruire les anciens charts
  if (attendanceChart) attendanceChart.destroy();
  if (attendancePieChart) attendancePieChart.destroy();

  // --- CHART LINE ---
  const ctxLine = lineCanvas.getContext("2d");
  attendanceChart = new Chart(ctxLine, {
    type: "line",
    data: {
      labels: ["Présents", "Absents", "Retards"],
      datasets: [{
        label: "Statistiques du jour",
        data: [data.presents ?? 0, data.absents ?? 0, data.retard ?? 0],
        backgroundColor: "rgba(54, 162, 235, 0.2)",
        borderColor: "rgba(54, 162, 235, 1)",
        borderWidth: 2,
        fill: true,
        tension: 0.4
      }]
    },
    options: { responsive: true }
  });

  // --- CHART PIE ---
  const ctxPie = pieCanvas.getContext("2d");
  attendancePieChart = new Chart(ctxPie, {
    type: "pie",
    data: {
      labels: ["Présents", "Absents", "Retards"],
      datasets: [{
        label: "Répartition",
        data: [
          data.pourcentage_presents ?? 0,
          data.pourcentage_absents ?? 0,
          data.pourcentage_retards ?? 0
        ],
        backgroundColor: [
          "rgba(54, 162, 235, 0.7)",
          "rgba(255, 99, 132, 0.7)",
          "rgba(255, 206, 86, 0.7)"
        ],
        borderColor: [
          "rgba(54, 162, 235, 1)",
          "rgba(255, 99, 132, 1)",
          "rgba(255, 206, 86, 1)"
        ],
        borderWidth: 1
      }]
    },
    options: { responsive: true, plugins: { legend: { position: "bottom" } } }
  });
}

document.addEventListener("DOMContentLoaded", () => {
  chargerDonneesDashboard();
  setInterval(chargerDonneesDashboard, 5000); // recharge toutes les 5s
});
