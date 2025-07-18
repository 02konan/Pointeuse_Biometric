document.addEventListener("DOMContentLoaded", () => {
  fetch("/api/liste_rapports").then(res => res.json()).then(liste => {
    liste.forEach(ajouterLigneRapport);
  });

  function ajouterLigneRapport(data) {
    const ligne = document.createElement("tr");
    ligne.innerHTML = `
      <td>${data.nom}</td>
      <td>${data.type}</td>
      <td>${data.periode}</td>
      <td>${data.auteur}</td>
      <td>${data.date}</td>
      <td>
        <a href="/telechargement/${encodeURIComponent(
          data.nom
        )}" class="btn btn-sm btn-outline-primary me-1">
          <i class="fas fa-download"></i>
        </a>
        <a href="/impression/${encodeURIComponent(
          data.nom
        )}" target="_blank" class="btn btn-sm btn-outline-secondary me-1">
          <i class="fas fa-print"></i>
        </a>
        <button onclick="supprimerRapport('${data.nom}', this)" class="btn btn-sm btn-outline-danger">
          <i class="fas fa-trash"></i>
        </button>
      </td>
    `;
    document.getElementById("table-rapports").prepend(ligne);
  }

  window.supprimerRapport = function(nom, btn) {
    fetch(`/suppression/${encodeURIComponent(nom)}`, { method: "DELETE" })
      .then(res => res.json())
      .then(data => {
        if (data.success) {
          const ligne = btn.closest("tr");
          ligne.remove();
        }
      });
  };
  document
    .getElementById("btn-fiche-presence")
    .addEventListener("click", () => {
      const dateDebut = document.getElementById("date_debut").value;
      const dateFin = document.getElementById("date_fin").value;

      fetch("/api/fiche_presence", {
        method: "POST",
        headers: {
          "Content-Type": "application/json"
        },
        body: JSON.stringify({
          date_debut: dateDebut,
          date_fin: dateFin
        })
      });
    });
    document.getElementById("btn-fiche-retards")
    .addEventListener("click", () => {
      const dateDebutretards = document.getElementById("date_debut_retard").value;
      const dateFinretards = document.getElementById("date_fin_retard").value;

      fetch("/api/fiche_retards", {
        method: "POST",
        headers: {
          "Content-Type": "application/json"
        },
        body: JSON.stringify({
          date_debut_retard: dateDebutretards,
          date_fin_retard: dateFinretards
        })
      });
    });

    document.getElementById("form-fiche-absences")
    .addEventListener("click", () => {
      const dateDebutabsence = document.getElementById("date_debut_absence").value;
      const dateFinabsence = document.getElementById("date_fin_absence").value;

      fetch("/api/fiche_absence", {
        method: "POST",
        headers: {
          "Content-Type": "application/json"
        },
        body: JSON.stringify({
          date_debut_absence: dateDebutabsence,
          date_fin_absence: dateFinabsence
        })
      });
    });
     document.getElementById("form-fiche-persornalise")
    .addEventListener("click", () => {
      const Matricule = document.getElementById("matricule").value;
      fetch("/api/fiche_presence_unique", {
        method: "POST",
        headers: {
          "Content-Type": "application/json"
        },
        body: JSON.stringify({
          Matricule: Matricule
        })
      });
    });
});
