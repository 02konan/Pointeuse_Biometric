function chargerDonneesDashboard() {
  fetch("/api/dashboard")
    .then(res => res.json())
    .then(data => {
      afficherActivites(data["activité_recentes"]);
      afficherCharts(data);

       // MISE À JOUR DES COMPTEURS QUOTIDIENS (existants)
      document.getElementById("presents-count").textContent = data.presents;
      document.getElementById("absents-count").textContent = data.absents;
      document.getElementById("retard-count").textContent = data.retard;
      document.getElementById("total-eleves").textContent = data.total_eleves;

      // MISE À JOUR DES BARRES DE PROGRESSION QUOTIDIENNES (existantes)
      document.getElementById("bar-present").style.width = data.pourcentage_presents + "%";
      document.getElementById("bar-absent").style.width = data.pourcentage_absents + "%";
      document.getElementById("bar-retard").style.width = data.pourcentage_retards + "%";

      // NOUVEAUX COMPTEURS MENSUELS (selon vos IDs HTML)
      if (document.getElementById("presence-mois-count")) {
        document.getElementById("presence-mois-count").textContent = data.employes_actifs_mois;
      }
      if (document.getElementById("absence-mois-count")) {
        // Calculer les absents du mois = Total - Actifs ce mois
        const absents_mois = data.total_eleves - data.employes_actifs_mois;
        document.getElementById("absence-mois-count").textContent = absents_mois;
      }
      if (document.getElementById("retard-mois-count")) {
        document.getElementById("retard-mois-count").textContent = data.employes_retard_mois;
      }
      if (document.getElementById("new-employee-count")) {
        // Pour l'instant, mettre 0 ou une valeur par défaut
        // Il faudra ajouter cette donnée dans votre API si nécessaire
        document.getElementById("new-employee-count").textContent = "0";
      }

      // NOUVELLES BARRES DE PROGRESSION MENSUELLES (selon vos IDs HTML)
      if (document.getElementById("bar-presence-mois")) {
        document.getElementById("bar-presence-mois").style.width = data.pourcentage_actifs_mois + "%";
      }
      if (document.getElementById("bar-absence-mois")) {
        document.getElementById("bar-absence-mois").style.width = data.pourcentage_inactifs_mois + "%";
      }
      if (document.getElementById("bar-retard-mois")) {
        document.getElementById("bar-retard-mois").style.width = data.pourcentage_retards_mois + "%";
      }
      if (document.getElementById("bar-new-employee")) {
        // Barre pour nouveaux employés (à 0% pour l'instant)
        document.getElementById("bar-new-employee").style.width = "0%";
      }
    })
    .catch(error =>
      console.error("Erreur lors du chargement du dashboard :", error)
    );
}

function afficherActivites(activites) {
  const container = document.getElementById("recent-activity-list");
  container.innerHTML = "";

  activites.forEach(([id, date]) => {
    const heures = new Date(date).getHours();
    const minutes = new Date(date).getMinutes();
    let status = "";
    let color="";
    if (heures < 16 || (heures === 16 && minutes >= 30)) {
      status = "Arrivée enregistrée";
      color = "text-success";
    } else if((heures === 16 && minutes >= 31) || (heures === 17 && minutes >= 30)){
      status = "Départ enregistré";
      color = "text-danger";
    }

    const item = document.createElement("li");
    item.className =
      "list-group-item d-flex justify-content-between align-items-center p-3";
    item.innerHTML = `
      <div class="d-flex align-items-center">
        <div class="avatar me-3">
          <img src="/placeholder.svg?height=40&width=40" alt="User">
        </div>
        <div>
          <h6 class="mb-0">${id}</h6>
          <small class="${color}">${status}</small>
        </div>
      </div>
      <span class="text-muted">${new Date(date).toLocaleString()}</span>
    `;
    container.appendChild(item);
  });
}

// Fonction pour afficher les graphiques
function afficherCharts(data) {
 
  const ctxLine = document.getElementById("attendanceChart").getContext("2d");
  new Chart(ctxLine, {
    type: "line",
    data: {
      labels: ["Présents", "Absents", "Retards"],
      datasets: [
        {
          label: "Statistiques du jour",
          data: [data.presents, data.absents, data.retard],
          backgroundColor: "rgba(54, 162, 235, 0.2)",
          borderColor: "rgba(54, 162, 235, 1)",
          borderWidth: 2,
          fill: true,
          tension: 0.4
        }
      ]
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

 
  const ctxPie = document.getElementById("attendancePieChart").getContext("2d");
  new Chart(ctxPie, {
    type: "pie",
    data: {
      labels: ["Présents", "Absents", "Retards"],
      datasets: [
        {
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
        }
      ]
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
  setInterval(chargerDonneesDashboard, 5000); // recharge toutes les 5s
});
