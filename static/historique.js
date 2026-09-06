/* =====================================================================
   BiometricWeb — filtres de l'historique des pointages
   Filtre par recherche, section, statut et intervalle de dates, puis
   délègue l'affichage à la pagination (évènement « bw:filtre »).
   ===================================================================== */
(function () {
  "use strict";

  // Accepte YYYY-MM-DD, DD/MM/YYYY et DD-MM-YYYY (format rendu par Flask).
  function parseDate(str) {
    if (!str) return null;
    str = String(str).trim();
    let m = str.match(/^(\d{4})-(\d{2})-(\d{2})$/);
    if (m) return new Date(+m[1], +m[2] - 1, +m[3]);
    m = str.match(/^(\d{2})[/-](\d{2})[/-](\d{4})$/);
    if (m) return new Date(+m[3], +m[2] - 1, +m[1]);
    return null;
  }

  function finDeJournee(d) {
    if (!d) return null;
    const copie = new Date(d);
    copie.setHours(23, 59, 59, 999);
    return copie;
  }

  document.addEventListener("DOMContentLoaded", function () {
    const corps = document.getElementById("dashboard_recent_activity_list");
    if (!corps) return;
    const table = corps.closest("table");

    const champs = {
      debut: document.getElementById("date-debut"),
      fin: document.getElementById("date-fin"),
      statut: document.getElementById("statut"),
      section: document.getElementById("section"),
      recherche: document.getElementById("champrecherche"),
    };

    function filtrer() {
      const debut = parseDate(champs.debut && champs.debut.value);
      const fin = finDeJournee(parseDate(champs.fin && champs.fin.value));
      const statut = champs.statut ? champs.statut.value : "tous";
      const section = champs.section ? champs.section.value : "toutes";
      const recherche = champs.recherche
        ? champs.recherche.value.trim().toLowerCase()
        : "";

      corps.querySelectorAll("tr").forEach(function (row) {
        if (row.dataset.emptyState === "true") return;
        // Colonnes : 0=Nom 1=Date 2=Heure 3=Statut 4=Section 5=Action
        const nom = row.cells[0] ? row.cells[0].textContent.trim() : "";
        const dateLigne = parseDate(row.cells[1] && row.cells[1].textContent);
        const statutLigne = row.cells[3] ? row.cells[3].textContent.trim() : "";
        const sectionLigne = row.cells[4] ? row.cells[4].textContent.trim() : "";

        let afficher = true;
        if (statut !== "tous" && statutLigne !== statut) afficher = false;
        if (section !== "toutes" && sectionLigne !== section) afficher = false;
        if (debut && (!dateLigne || dateLigne < debut)) afficher = false;
        if (fin && (!dateLigne || dateLigne > fin)) afficher = false;
        if (recherche && !nom.toLowerCase().includes(recherche)) afficher = false;

        row.dataset.filtered = afficher ? "" : "hidden";
      });

      document.dispatchEvent(
        new CustomEvent("bw:filtre", { detail: { table: table } })
      );
    }

    Object.values(champs).forEach(function (el) {
      if (!el) return;
      el.addEventListener(el.tagName === "INPUT" && el.type === "text" ? "input" : "change", filtrer);
    });

    // Bouton de réinitialisation optionnel.
    const reset = document.getElementById("reset-filtres");
    if (reset) {
      reset.addEventListener("click", function () {
        if (champs.debut) champs.debut.value = "";
        if (champs.fin) champs.fin.value = "";
        if (champs.statut) champs.statut.value = "tous";
        if (champs.section) champs.section.value = "toutes";
        if (champs.recherche) champs.recherche.value = "";
        filtrer();
      });
    }

    filtrer();
  });
})();
