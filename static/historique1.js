
// Fonction pour filtrer le tableau
function filtrerActivites() {
    const dateDebut = document.getElementById('date-debut').value;
    const dateFin = document.getElementById('date-fin').value;
    const statut = document.getElementById('statut').value;

    const rows = document.querySelectorAll('#dashboard_recent_activity_list tr');

    rows.forEach(row => {
        const dateCell = row.cells[3].textContent; // colonne Date
        const statutCell = row.cells[2].textContent; // colonne Statut

        let afficher = true;

        // Filtrer par statut
        if (statut !== 'tous' && statutCell !== statut) {
            afficher = false;
        }

        // Filtrer par date
        if (dateDebut && dateCell < dateDebut) {
            afficher = false;
        }
        if (dateFin && dateCell > dateFin) {
            afficher = false;
        }

        row.style.display = afficher ? '' : 'none';
    });
}

// Ajouter les écouteurs d'événements
document.getElementById('date-debut').addEventListener('change', filtrerActivites);
document.getElementById('date-fin').addEventListener('change', filtrerActivites);
document.getElementById('statut').addEventListener('change', filtrerActivites);

