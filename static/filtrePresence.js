/* =====================================================================
   BiometricWeb — filtres de la page « Pointage valide »
   Recherche libre, intervalle de dates et statut. La pagination gère
   ensuite le découpage en pages et le nombre de lignes affichées.
   ===================================================================== */
(function () {
  "use strict";

  function parseDate(str) {
    if (!str) return null;
    str = String(str).trim();
    let m = str.match(/^(\d{4})-(\d{2})-(\d{2})$/);
    if (m) return new Date(+m[1], +m[2] - 1, +m[3]);
    m = str.match(/^(\d{2})[/-](\d{2})[/-](\d{4})$/);
    if (m) return new Date(+m[3], +m[2] - 1, +m[1]);
    return null;
  }

  document.addEventListener("DOMContentLoaded", function () {
    const table = document.getElementById("Table");
    if (!table) return;

    const statutSelect = document.getElementById("filtre-statut");
    const dateDebut = document.getElementById("filtre-date-debut");
    const dateFin = document.getElementById("filtre-date-fin");
    const recherche = document.getElementById("filtre-recherche");

    function filtrer() {
      const debut = parseDate(dateDebut && dateDebut.value);
      const finBrute = parseDate(dateFin && dateFin.value);
      const fin = finBrute ? new Date(finBrute.setHours(23, 59, 59, 999)) : null;
      const statut = statutSelect ? statutSelect.value : "tous";
      const q = recherche ? recherche.value.trim().toLowerCase() : "";

      table.querySelectorAll("tbody tr").forEach(function (row) {
        if (row.dataset.emptyState === "true") return;
        // Colonnes : 0=ID 1=Employé 2=Date 3=Jour 4=Arrivée 5=Départ
        //            6=Durée prévue 7=Durée effectuée 8=Statut
        const dateLigne = parseDate(row.cells[2] && row.cells[2].textContent);
        const statutLigne = row.cells[8] ? row.cells[8].textContent.trim() : "";

        let afficher = true;
        if (statut !== "tous" && statutLigne !== statut) afficher = false;
        if (debut && (!dateLigne || dateLigne < debut)) afficher = false;
        if (fin && (!dateLigne || dateLigne > fin)) afficher = false;
        if (q && !row.textContent.toLowerCase().includes(q)) afficher = false;

        row.dataset.filtered = afficher ? "" : "hidden";
      });

      document.dispatchEvent(
        new CustomEvent("bw:filtre", { detail: { table: table } })
      );
    }

    [statutSelect, dateDebut, dateFin].forEach(function (el) {
      if (el) el.addEventListener("change", filtrer);
    });
    if (recherche) recherche.addEventListener("input", filtrer);

    const reset = document.getElementById("reset-filtres");
    if (reset) {
      reset.addEventListener("click", function () {
        [dateDebut, dateFin, recherche].forEach(function (el) {
          if (el) el.value = "";
        });
        if (statutSelect) statutSelect.value = "tous";
        filtrer();
      });
    }

    filtrer();
  });
})();
