document.addEventListener("DOMContentLoaded", () => {

  const sidebarToggle = document.getElementById("sidebar-toggle")
  if (sidebarToggle) {
    sidebarToggle.addEventListener("click", () => {
      const sidebar = document.querySelector(".sidebar")
      const mainContent = document.querySelector(".main-content")

      sidebar.classList.toggle("expanded")
      mainContent.classList.toggle("sidebar-expanded")
    })
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

  // Initialize Charts
  initCharts()
})

function initCharts() {
  // Attendance Chart
  const attendanceChartEl = document.getElementById("attendanceChart")
  if (attendanceChartEl) {
    new Chart(attendanceChartEl, {
      type: "line",
      data: {
        labels: ["Lun", "Mar", "Mer", "Jeu", "Ven", "Sam", "Dim"],
        datasets: [
          {
            label: "Présences",
            data: [42, 45, 40, 43, 38, 20, 15],
            borderColor: "#4361ee",
            backgroundColor: "rgba(67, 97, 238, 0.1)",
            tension: 0.3,
            fill: true,
          },
          {
            label: "Retards",
            data: [5, 3, 7, 4, 6, 2, 1],
            borderColor: "#ff9800",
            backgroundColor: "rgba(255, 152, 0, 0.1)",
            tension: 0.3,
            fill: true,
          },
        ],
      },
      options: {
        responsive: true,
        plugins: {
          legend: {
            position: "top",
          },
          title: {
            display: false,
          },
        },
        scales: {
          y: {
            beginAtZero: true,
          },
        },
      },
    })
  }

  // Attendance Pie Chart
  const attendancePieChartEl = document.getElementById("attendancePieChart")
  if (attendancePieChartEl) {
    new Chart(attendancePieChartEl, {
      type: "doughnut",
      data: {
        labels: ["Présents", "Retards", "Absents"],
        datasets: [
          {
            data: [42, 5, 8],
            backgroundColor: ["#4caf50", "#ff9800", "#f44336"],
            borderWidth: 0,
          },
        ],
      },
      options: {
        responsive: true,
        plugins: {
          legend: {
            position: "bottom",
          },
        },
        cutout: "70%",
      },
    })
  }
}
