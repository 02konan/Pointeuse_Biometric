function chargerDonneesDashboard() {
  fetch("/api/dashboard")
    .then(res => res.json())
    .then(data => {
      afficherActivites(data["activité_recentes"]);
      afficherCharts(data);

      // Mise à jour des compteurs
      document.getElementById("presents-count").textContent = data.presents;
      document.getElementById("absents-count").textContent = data.absents;
      document.getElementById("retard-count").textContent = data.retard;
      document.getElementById("total-eleves").textContent = data.total_eleves;

      // Mise à jour des barres de progression
      document.getElementById("bar-present").style.width = data.pourcentage_presents + "%";
      document.getElementById("bar-absent").style.width = data.pourcentage_absents + "%";
      document.getElementById("bar-retard").style.width = data.pourcentage_retards + "%";
    })
    .catch(error => console.error("Erreur lors du chargement du dashboard :", error));
}

// Fonction pour afficher les activités récentes
function afficherActivites(activites) {
  const container = document.getElementById("recent-activity-list");
  container.innerHTML = ""; // Nettoyer la liste

  activites.forEach(([id, date]) => {
    const item = document.createElement("li");
    item.className = "list-group-item d-flex justify-content-between align-items-center p-3";
    item.innerHTML = `
      <div class="d-flex align-items-center">
        <div class="avatar me-3">
          <img src="/placeholder.svg?height=40&width=40" alt="User">
        </div>
        <div>
          <h6 class="mb-0">${id}</h6>
          <small class="text-muted">Arrivée enregistrée</small>
        </div>
      </div>
      <span class="text-muted">${new Date(date).toLocaleString()}</span>
    `;
    container.appendChild(item);
  });
}

// Fonction pour afficher les graphiques
function afficherCharts(data) {
  // === Ligne ===
  const ctxLine = document.getElementById("attendanceChart").getContext("2d");
  new Chart(ctxLine, {
    type: "line",
    data: {
      labels: ["Présents", "Absents", "Retards"],
      datasets: [{
        label: "Statistiques du jour",
        data: [data.presents, data.absents, data.retard],
        backgroundColor: "rgba(54, 162, 235, 0.2)",
        borderColor: "rgba(54, 162, 235, 1)",
        borderWidth: 2,
        fill: true,
        tension: 0.4
      }]
    },
    options: {
      responsive: true,
      plugins: {
        legend: {
          display: true
        }
      }
    }
  });

  // === Camembert ===
  const ctxPie = document.getElementById("attendancePieChart").getContext("2d");
  new Chart(ctxPie, {
    type: "pie",
    data: {
      labels: ["Présents", "Absents", "Retards"],
      datasets: [{
        label: "Répartition",
        data: [
          data.pourcentage_presents,
          data.pourcentage_absents,
          data.pourcentage_retards
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
    options: {
      responsive: true,
      plugins: {
        legend: {
          position: "bottom"
        }
      }
    }
  });
}

// Lancer une fois au chargement
document.addEventListener("DOMContentLoaded", () => {
  chargerDonneesDashboard();

  // Rafraîchissement toutes les 30 secondes
  setInterval(chargerDonneesDashboard, 10000);
});