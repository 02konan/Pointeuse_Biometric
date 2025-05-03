document.addEventListener("DOMContentLoaded", () => {

  const sidebarToggle = document.getElementById("sidebar-toggle")
  if (sidebarToggle) {
    sidebarToggle.addEventListener("click", () => {
      const sidebar = document.querySelector(".sidebar")
      const mainContent = document.querySelector(".main-content")

      sidebar.classList.toggle("expanded")
      mainContent.classList.toggle("sidebar-expanded")
    })

    sidebarToggle.addEventListener("click", function () {
      const sidebar = document.querySelector('.sidebar');
      sidebar.classList.toggle('open');
    });

    sidebarToggle.addEventListener("click", function () {
      const sidebar = document.querySelector('.sidebar');
      sidebar.classList.toggle('collapsed');
    });

    sidebarToggle.addEventListener("click", function () {
      const sidebar = document.querySelector('.sidebar');
      sidebar.classList.toggle('collapsed');
    });
  }

  // Sidebar Menu Navigation
  const sidebarMenuItems = document.querySelectorAll(".sidebar-menu li")
  if (sidebarMenuItems.length > 0) {
    sidebarMenuItems.forEach((item) => {
      item.addEventListener("click", function () {
        // Remove active class from all items
        sidebarMenuItems.forEach((i) => i.classList.remove("active"))

        // Add active class to clicked item
        this.classList.add("active")

        // Hide all content sections
        document.querySelectorAll(".content-section").forEach((section) => {
          section.classList.add("d-none")
        })

        // Show the target content section
        const targetId = this.getAttribute("data-target")
        document.getElementById(targetId).classList.remove("d-none")
      })
    })
  }

  // Logout Button
  const logoutBtn = document.getElementById("logout-btn")
  const dropdownLogout = document.getElementById("dropdown-logout")

  if (logoutBtn) {
    logoutBtn.addEventListener("click", handleLogout)
  }

  if (dropdownLogout) {
    dropdownLogout.addEventListener("click", handleLogout)
  }

  function handleLogout(e) {
    e.preventDefault()
    document.getElementById("dashboard").classList.add("d-none")
    document.getElementById("login-page").classList.remove("d-none")
  }
  
 
})

