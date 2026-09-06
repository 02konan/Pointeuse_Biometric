/* =====================================================================
   BiometricWeb — socle d'interface
   Sidebar, thème clair/sombre, toasts, horloge, recherche de tableau.
   Toutes les fonctions sont défensives : une page qui ne contient pas
   un élément n'interrompt plus l'exécution des autres modules.
   ===================================================================== */
(function () {
  "use strict";

  /* ---------------- Sidebar ---------------- */
  function initSidebar() {
    const sidebar = document.getElementById("sidebar");
    const toggle = document.getElementById("sidebar-toggle");
    const backdrop = document.getElementById("sidebar-backdrop");
    const main = document.getElementById("main-content");
    if (!sidebar || !toggle) return;

    const MOBILE = "(max-width: 991.98px)";
    const estMobile = () => window.matchMedia(MOBILE).matches;

    function fermerMobile() {
      sidebar.classList.remove("open");
      if (backdrop) backdrop.classList.remove("show");
      document.body.style.overflow = "";
    }

    // Restaure l'état replié choisi précédemment (desktop uniquement).
    try {
      if (localStorage.getItem("bw-sidebar") === "collapsed" && !estMobile()) {
        sidebar.classList.add("collapsed");
        if (main) main.classList.add("expanded");
      }
    } catch (e) {}

    toggle.addEventListener("click", function () {
      if (estMobile()) {
        const ouvert = sidebar.classList.toggle("open");
        if (backdrop) backdrop.classList.toggle("show", ouvert);
        document.body.style.overflow = ouvert ? "hidden" : "";
        return;
      }
      const replie = sidebar.classList.toggle("collapsed");
      if (main) main.classList.toggle("expanded", replie);
      try {
        localStorage.setItem("bw-sidebar", replie ? "collapsed" : "expanded");
      } catch (e) {}
    });

    if (backdrop) backdrop.addEventListener("click", fermerMobile);

    document.addEventListener("keydown", function (e) {
      if (e.key === "Escape") fermerMobile();
    });

    window.addEventListener("resize", function () {
      if (!estMobile()) fermerMobile();
    });
  }

  /* ---------------- Thème clair / sombre ---------------- */
  function initTheme() {
    const bouton = document.getElementById("theme-toggle");
    const racine = document.documentElement;

    function peindre() {
      const sombre = racine.getAttribute("data-bs-theme") === "dark";
      if (bouton) {
        bouton.innerHTML = sombre
          ? '<i class="fas fa-sun"></i>'
          : '<i class="fas fa-moon"></i>';
        bouton.setAttribute(
          "title",
          sombre ? "Passer en thème clair" : "Passer en thème sombre"
        );
      }
      document.dispatchEvent(new CustomEvent("bw:theme", { detail: { sombre } }));
    }

    if (bouton) {
      bouton.addEventListener("click", function () {
        const sombre = racine.getAttribute("data-bs-theme") === "dark";
        const suivant = sombre ? "light" : "dark";
        racine.setAttribute("data-bs-theme", suivant);
        try {
          localStorage.setItem("bw-theme", suivant);
        } catch (e) {}
        peindre();
      });
    }
    peindre();
  }

  /* ---------------- Toasts ---------------- */
  function initToasts() {
    if (typeof bootstrap === "undefined") return;
    document.querySelectorAll(".toast").forEach(function (el) {
      try {
        bootstrap.Toast.getOrCreateInstance(el).show();
      } catch (e) {}
    });
  }

  // Notification programmatique, réutilisable par les autres scripts.
  window.bwToast = function (message, type) {
    let zone = document.getElementById("flash-container");
    if (!zone) {
      zone = document.createElement("div");
      zone.id = "flash-container";
      zone.className = "toast-container bw-toasts";
      document.body.appendChild(zone);
    }
    const ton = type === "success" ? "success" : type === "warning" ? "warning" : "danger";
    const icone = ton === "success" ? "fa-circle-check" : "fa-triangle-exclamation";
    const el = document.createElement("div");
    el.className = "toast align-items-center text-bg-" + ton + " border-0";
    el.setAttribute("role", "alert");
    el.innerHTML =
      '<div class="d-flex"><div class="toast-body"><i class="fas ' +
      icone +
      ' me-2"></i>' +
      message +
      '</div><button type="button" class="btn-close btn-close-white me-2 m-auto" data-bs-dismiss="toast" aria-label="Fermer"></button></div>';
    zone.appendChild(el);
    if (typeof bootstrap !== "undefined") {
      const t = bootstrap.Toast.getOrCreateInstance(el, { delay: 6000 });
      el.addEventListener("hidden.bs.toast", () => el.remove());
      t.show();
    }
  };

  /* ---------------- Horloge de la barre supérieure ---------------- */
  function initClock() {
    const cible = document.getElementById("topbar-date");
    if (!cible) return;
    function afficher() {
      const d = new Date();
      const date = d.toLocaleDateString("fr-FR", {
        weekday: "short",
        day: "2-digit",
        month: "short",
      });
      const heure = d.toLocaleTimeString("fr-FR", {
        hour: "2-digit",
        minute: "2-digit",
      });
      cible.textContent = date + " · " + heure;
    }
    afficher();
    setInterval(afficher, 30000);
  }

  /* ---------------- Masquage du loader ---------------- */
  function initLoader() {
    const loader = document.getElementById("loader");
    if (!loader) return;
    const cacher = () => loader.classList.add("is-hidden");
    if (document.readyState === "complete") cacher();
    else window.addEventListener("load", cacher);
    // Filet de sécurité si une ressource externe ne répond pas.
    setTimeout(cacher, 4000);
  }

  /* ---------------- Recherche générique dans un tableau ----------------
     Utilisée par la page Employés (#champrecherche + #Table) et par toute
     page déclarant data-table-search="<id du tableau>".
     ------------------------------------------------------------------- */
  function initRechercheTableau() {
    const champs = [];
    const champEmploye = document.getElementById("champrecherche");
    const table = document.getElementById("Table");
    // La page Historique gère sa propre logique de filtrage.
    if (champEmploye && table && !document.getElementById("statut")) {
      champs.push([champEmploye, table]);
    }
    document.querySelectorAll("[data-table-search]").forEach(function (input) {
      const cible = document.getElementById(input.getAttribute("data-table-search"));
      if (cible) champs.push([input, cible]);
    });

    champs.forEach(function ([input, tableau]) {
      input.addEventListener("input", function () {
        const q = input.value.trim().toLowerCase();
        tableau.querySelectorAll("tbody tr").forEach(function (tr) {
          if (tr.dataset.emptyState === "true") return;
          const visible = !q || tr.textContent.toLowerCase().includes(q);
          tr.dataset.filtered = visible ? "" : "hidden";
        });
        document.dispatchEvent(
          new CustomEvent("bw:filtre", { detail: { table: tableau } })
        );
      });
    });
  }

  /* ---------------- Modale employé (ajout / modification) ---------------- */
  function initModaleEmploye() {
    const modal = document.getElementById("addEmployeeModal");
    if (!modal) return;
    const form = modal.querySelector("form");
    const titre = modal.querySelector(".modal-title");
    if (!form) return;

    function remplir(champ, valeur) {
      if (form.elements[champ]) form.elements[champ].value = valeur || "";
    }

    modal.addEventListener("show.bs.modal", function (event) {
      const btn = event.relatedTarget;
      const modification = btn && btn.hasAttribute("data-id");
      if (titre) {
        titre.textContent = modification
          ? "Modifier l'employé"
          : "Ajouter un nouvel employé";
      }
      if (!modification) {
        form.reset();
        return;
      }
      remplir("idEmploye", btn.getAttribute("data-id"));
      remplir("section", btn.getAttribute("data-section"));
      remplir("email", btn.getAttribute("data-email"));
      remplir("telephone", btn.getAttribute("data-telephone"));
      remplir("position", btn.getAttribute("data-poste"));
      remplir("address", btn.getAttribute("data-adresse"));
      remplir("joinDate", btn.getAttribute("data-date"));
    });
  }

  /* ---------------- Démarrage ---------------- */
  document.addEventListener("DOMContentLoaded", function () {
    initLoader();
    initSidebar();
    initTheme();
    initToasts();
    initClock();
    initRechercheTableau();
    initModaleEmploye();
  });
})();
