function filtrerActivites() {

    const dateDebutStr = document.getElementById('date-debut') ? document.getElementById('date-debut').value : '';
    const dateFinStr = document.getElementById('date-fin') ? document.getElementById('date-fin').value : '';
    const statut = document.getElementById('statut') ? document.getElementById('statut').value : 'tous';
    const section = document.getElementById('section') ? document.getElementById('section').value : 'toutes';
    const recherchetable = document.getElementById('champrecherche') ? document.getElementById('champrecherche').value : '';

    const rows = document.querySelectorAll('#dashboard_recent_activity_list tr');

    rows.forEach(row => {

        const nomCell = row.cells[0] ? row.cells[0].textContent.trim() : '';
        const statutCell = row.cells[3] ? row.cells[3].textContent.trim() : '';
        const sectionCell = row.cells[4] ? row.cells[4].textContent.trim() : '';
        const dateText = row.children[1]?.textContent.trim();

        let rowDate = null;
      if (dateText.includes("/")) {
        const parts = dateText.split("/");
        rowDate = new Date(parts[2], parts[1] - 1, parts[0]);
      } else if (dateText.includes("-")) {
        const parts = dateText.split("-");
        rowDate = new Date(parts[0], parts[1] - 1, parts[2]);
      }


        
        let afficher = true;

        // Filtrer par statut
        if (statut !== 'tous' && statutCell !== statut) {
            afficher = false;
        }

        // Filtrer par date (utiliser objets Date pour comparaison)
        if (dateDebutStr && dateCellRaw< dateDebutStr) {
            afficher = false;
        }
        if (dateFinStr && dateCellRaw> dateFinStr) {
            afficher = false;
        }
        

        // Filtrer par section
        if (section !== 'toutes' && sectionCell !== section) {
            afficher = false;
        }

        // Filtrer par recherche (insensible à la casse)
        if (recherchetable && !nomCell.toLowerCase().includes(recherchetable.toLowerCase().trim())) {
            afficher = false;
        }

        row.style.display = afficher ? '' : 'none';
    });
}

// Attacher les écouteurs d'événements lorsque le DOM est prêt
document.addEventListener('DOMContentLoaded', () => {
    const dateDebutEl = document.getElementById('date-debut');
    const dateFinEl = document.getElementById('date-fin');
    const statutEl = document.getElementById('statut');
    const sectionEl = document.getElementById('section');
    const rechercheEl = document.getElementById('champrecherche');

    if (dateDebutEl) dateDebutEl.addEventListener('change', filtrerActivites);
    if (dateFinEl) dateFinEl.addEventListener('change', filtrerActivites);
    if (statutEl) statutEl.addEventListener('change', filtrerActivites);
    if (sectionEl) sectionEl.addEventListener('change', filtrerActivites);
    if (rechercheEl) rechercheEl.addEventListener('input', filtrerActivites);

    // Lancer un premier filtrage pour appliquer les valeurs initiales
    filtrerActivites();
});

