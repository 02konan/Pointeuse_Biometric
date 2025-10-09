document.addEventListener("DOMContentLoaded", function() {
  const table = document.getElementById("Table");
  const statutSelect = document.querySelector("select.form-select");
  const dateInputs = document.querySelectorAll("input[type='date']");

  function filterTable() {
    const debut = dateInputs[0].value ? new Date(dateInputs[0].value) : null;
    const fin = dateInputs[1].value ? new Date(dateInputs[1].value) : null;
    const statut = statutSelect.value;

    const rows = table.querySelectorAll("tbody tr");
    rows.forEach(row => {
      // Récupère la date
      const dateText = row.children[2]?.textContent.trim(); // "dd/mm/yyyy" ou "yyyy-mm-dd"
      let rowDate = null;
      if (dateText.includes("/")) {
        const parts = dateText.split("/");
        rowDate = new Date(parts[2], parts[1] - 1, parts[0]);
      } else if (dateText.includes("-")) {
        const parts = dateText.split("-");
        rowDate = new Date(parts[0], parts[1] - 1, parts[2]);
      }

      // Filtre date
      let matchDate = true;
      if (debut && rowDate && rowDate < debut) matchDate = false;
      if (fin && rowDate && rowDate > fin) matchDate = false;

      // Filtre statut
      const rowStatut = row.children[8]?.textContent.trim();
      let matchStatut = (statut === 'Tous les statuts') || (rowStatut === statut);

      row.style.display = (matchDate && matchStatut) ? '' : 'none';
    });
  }

  statutSelect.addEventListener("change", filterTable);
  dateInputs.forEach(input => input.addEventListener("change", filterTable));
});
