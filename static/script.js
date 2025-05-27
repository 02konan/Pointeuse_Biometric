document.addEventListener("DOMContentLoaded", () => {

  // Gestion du bouton pour ouvrir/fermer la sidebar
  const sidebarToggle = document.getElementById("sidebar-toggle");
  if (sidebarToggle) {
    sidebarToggle.addEventListener("click", () => {
      const sidebar = document.querySelector(".sidebar");
      const mainContent = document.querySelector(".main-content");

      // Toggle des classes nécessaires
      sidebar.classList.toggle("expanded");
      sidebar.classList.toggle("open");
      sidebar.classList.toggle("collapsed");
      mainContent.classList.toggle("sidebar-expanded");
    });
  }

  // Gestion du menu actif dans la sidebar
  const sidebarMenuItems = document.querySelectorAll(".sidebar-menu li a");
  if (sidebarMenuItems.length > 0) {
    sidebarMenuItems.forEach((item) => {
      item.addEventListener("click", function () {
        sidebarMenuItems.forEach((i) => i.classList.remove("active"));
        this.classList.add("active");
      });
    });
  }

  // Gestion de la déconnexion
  // const logoutBtn = document.getElementById("logout-btn");
  // const dropdownLogout = document.getElementById("dropdown-logout");

  // if (logoutBtn) {
  //   logoutBtn.addEventListener("click", handleLogout);
  // }

  // if (dropdownLogout) {
  //   dropdownLogout.addEventListener("click", handleLogout);
  // }

  // function handleLogout(e) {
  //   e.preventDefault();
  //   document.getElementById("dashboard").classList.add("d-none");
  //   document.getElementById("login-page").classList.remove("d-none");
  // }

  var modal = document.getElementById('addEmployeeModal');
  var form = modal.querySelector('form');

  // Quand on clique sur "Modifier"
  document.querySelectorAll('[data-bs-target="#addEmployeeModal"][data-id]').forEach(function(btn) {
    btn.addEventListener('click', function () {
      form.nom.value = btn.getAttribute('data-nom');
      form.prenom.value = btn.getAttribute('data-prenom');
      form.idEmploye.value = btn.getAttribute('data-id');
      form.section.value = btn.getAttribute('data-section');
      form.email.value = btn.getAttribute('data-email');
      form.telephone.value = btn.getAttribute('data-telephone');
      form.Poste.value = btn.getAttribute('data-Poste');
      form.date.value = btn.getAttribute('data-date');
      form.Adresse.value = btn.getAttribute('data-Adresse');
    });
  });

  // Réinitialise le formulaire à l'ouverture du modal pour ajout
  modal.addEventListener('show.bs.modal', function (event) {
    if (!event.relatedTarget || !event.relatedTarget.hasAttribute('data-id')) {
      form.reset();
    }
  });
});
