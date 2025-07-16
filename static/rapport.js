document.addEventListener("DOMContentLoaded", () => {
  fetch("/api/liste_rapports")
    .then(res => res.json())
    .then(liste => {
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
        <a href="/telechargement/${encodeURIComponent(data.nom)}" class="btn btn-sm btn-outline-primary me-1">
          <i class="fas fa-download"></i>
        </a>
        <a href="/impression/${encodeURIComponent(data.nom)}" target="_blank" class="btn btn-sm btn-outline-secondary me-1">
          <i class="fas fa-print"></i>
        </a>
        <button onclick="ouvrirModaleSuppression('${data.nom}', this)" class="btn btn-sm btn-outline-danger">
  <i class="fas fa-trash"></i>
</button>

      </td>
    `;
    document.getElementById("table-rapports").prepend(ligne);
  }

  let fichierASupprimer = null;
let boutonSupprimer = null;

function ouvrirModaleSuppression(nomFichier, btn) {
  fichierASupprimer = nomFichier;
  boutonSupprimer = btn;
  const modal = new bootstrap.Modal(document.getElementById('modalSuppression'));
  modal.show();
}

document.getElementById("confirmerSuppression").addEventListener("click", () => {
  if (!fichierASupprimer) return;

  fetch(`/suppression/${encodeURIComponent(fichierASupprimer)}`, {
    method: "DELETE"
  })
  .then(res => res.json())
  .then(data => {
    if (data.success) {
      const ligne = boutonSupprimer.closest("tr");
      ligne.remove();
      fichierASupprimer = null;
      boutonSupprimer = null;
      const modal = bootstrap.Modal.getInstance(document.getElementById('modalSuppression'));
      modal.hide();
    }
  });
});
});
document.getElementById("form-fiche-presence").addEventListener("submit", function (e) {
  e.preventDefault();

  const date_debut = document.getElementById("date_debut").value;
  const date_fin = document.getElementById("date_fin").value;

  fetch("/api/fiche_presence", {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify({
      date_debut,
      date_fin
    })
  })
  .then(response => response.json())
  .then(data => {
    if (data.success) {
      ajouterLigneRapport(data);
    } else {
      alert(data.error || "Erreur inconnue");
    }
  });
});