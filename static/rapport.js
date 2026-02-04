document.addEventListener("DOMContentLoaded", () => {

  fetch("/api/liste_rapports")
    .then(res => res.json())
    .then(liste => liste.forEach(ajouterLigneRapport));

  function ajouterLigneRapport(data) {
    const ligne = document.createElement("tr");
    ligne.innerHTML = `
      <td>${data.nom}</td>
      <td>${data.type}</td>
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

  // Suppression d’un rapport
  window.supprimerRapport = function(nom, btn) {
    if (!confirm("Voulez-vous vraiment supprimer ce rapport ?")) return;
    fetch(`/suppression/${encodeURIComponent(nom)}`, { method: "DELETE" })
      .then(res => res.json())
      .then(data => {
        if (data.success) {
          const ligne = btn.closest("tr");
          ligne.remove();
        }
      });
  };

  // Bouton de génération de rapport
  const btnGenerer = document.getElementById("btn-fiche-presence");
  btnGenerer.addEventListener("click", () => {
    const dateDebut = document.getElementById("date_debut").value;
    const dateFin = document.getElementById("date_fin").value;
    const employeeid = document.getElementById("idEmploye").value;

    if (!dateDebut || !dateFin) {
      alert("Veuillez sélectionner une date de début et une date de fin.");
      return;
    }

    // Afficher le spinner / message
    btnGenerer.innerHTML = '<i class="fas fa-spinner fa-spin me-2"></i>Génération en cours...';
    btnGenerer.disabled = true;

    fetch("/api/fiche_presence", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ date_debut: dateDebut, date_fin: dateFin, idEmploye: employeeid})
    })
    .then(async response => {
      const result = await response.json();
      if (!response.ok) throw new Error(result.error || "Erreur lors de la génération du rapport");
      ajouterLigneRapport({
        nom: result.nom,
        type: result.type,
        auteur: result.auteur,
        date: result.date
      });
      alert(`Rapport généré avec succès : ${result.nom}`);
    })
    .finally(() => {
      // Réinitialiser le bouton
      btnGenerer.innerHTML = '<i class="fas fa-file-pdf me-2"></i>Générer le rapport';
      btnGenerer.disabled = false;
    });
  });

});


    const btnGenereradmin = document.getElementById("btn-fiche-presence-admin");
    btnGenereradmin.addEventListener("click", () => {
      const dateDebut_admin = document.getElementById("date_debut_admin").value;
      const dateFin_admin = document.getElementById("date_fin_admin").value;
      const employeeid_admin = document.getElementById("idEmploye_admin").value;
      const section_admin =document.getElementById("sectionid").value;
      
      btnGenereradmin.innerHTML = '<i class="fas fa-spinner fa-spin me-2"></i>Génération en cours...';
      btnGenereradmin.disabled = true;

      fetch("/api/fiche_presence_admin", {
        method: "POST",
        headers: {
          "Content-Type": "application/json"
        },
        body: JSON.stringify({
          date_admin_debut: dateDebut_admin,
          date_admin_fin: dateFin_admin,
          employeeid_admin: employeeid_admin,
          section_admin: section_admin
        })
      });
    });

    // document.getElementById("form-fiche-absences")
    // .addEventListener("click", () => {
    //   const dateDebutabsence = document.getElementById("date_debut_absence").value;
    //   const dateFinabsence = document.getElementById("date_fin_absence").value;

    //   fetch("/api/fiche_absence", {
    //     method: "POST",
    //     headers: {
    //       "Content-Type": "application/json"
    //     },
    //     body: JSON.stringify({
    //       date_debut_absence: dateDebutabsence,
    //       date_fin_absence: dateFinabsence
    //     })
    //   });
    // });

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

