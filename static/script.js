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

});
