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
        <button onclick="supprimerRapport('${data.nom}', this)" class="btn btn-sm btn-outline-danger">
          <i class="fas fa-trash"></i>
        </button>
      </td>
    `;
    document.getElementById("table-rapports").prepend(ligne);
  }

  window.supprimerRapport = function(nom, btn) {
    if (!confirm("Voulez-vous vraiment supprimer ce fichier ?")) return;
    fetch(`/suppression/${encodeURIComponent(nom)}`, { method: "DELETE" })
      .then(res => res.json())
      .then(data => {
        if (data.success) {
          const ligne = btn.closest("tr");
          ligne.remove();
        }
      });
  };
});
