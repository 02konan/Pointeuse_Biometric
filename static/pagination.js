/* =====================================================================
   BiometricWeb — pagination des tableaux
   Seul responsable de la visibilité des lignes : une ligne est affichée
   si elle passe les filtres (data-filtered) ET appartient à la page
   courante. Les scripts de filtrage se contentent de marquer les lignes
   puis d'émettre l'évènement « bw:filtre ».
   ===================================================================== */
(function () {
  "use strict";

  const TAILLE_DEFAUT = 10;

  function creerPagination(table) {
    const tbody = table.querySelector("tbody");
    const conteneur = table.closest(".card-body") || table.parentElement;
    if (!tbody || !conteneur) return null;

    // Sélecteur du nombre de lignes, s'il existe sur la page.
    const selecteurTaille =
      document.getElementById("lignesaffichees") ||
      document.getElementById("itemsPerPage");

    let pagination = conteneur.querySelector(".pagination");
    let pied = conteneur.querySelector(".table-footer");
    if (!pied) {
      pied = document.createElement("div");
      pied.className = "table-footer";
      if (pagination && pagination.parentElement) {
        // Réutilise le <nav> déjà présent dans le gabarit.
        pied.appendChild(pagination.parentElement);
      } else {
        const nav = document.createElement("nav");
        nav.setAttribute("aria-label", "Pagination du tableau");
        pagination = document.createElement("ul");
        pagination.className = "pagination justify-content-center";
        nav.appendChild(pagination);
        pied.appendChild(nav);
      }
      conteneur.appendChild(pied);
    }
    if (!pagination) pagination = pied.querySelector(".pagination");

    let compteur = pied.querySelector(".table-count");
    if (!compteur) {
      compteur = document.createElement("div");
      compteur.className = "table-count";
      pied.insertBefore(compteur, pied.firstChild);
    }

    let pageCourante = 1;

    function lignes() {
      return Array.from(tbody.querySelectorAll("tr")).filter(
        (tr) => tr.dataset.emptyState !== "true"
      );
    }

    function taillePage() {
      const v = selecteurTaille ? parseInt(selecteurTaille.value, 10) : NaN;
      return Number.isFinite(v) && v > 0 ? v : TAILLE_DEFAUT;
    }

    function rendre() {
      const toutes = lignes();
      const visibles = toutes.filter((tr) => tr.dataset.filtered !== "hidden");
      const parPage = taillePage();
      const total = visibles.length;
      const pages = Math.max(1, Math.ceil(total / parPage));
      if (pageCourante > pages) pageCourante = pages;

      const debut = (pageCourante - 1) * parPage;
      const fin = debut + parPage;

      toutes.forEach((tr) => (tr.style.display = "none"));
      visibles.forEach((tr, i) => {
        tr.style.display = i >= debut && i < fin ? "" : "none";
      });

      // Ligne « aucun résultat » gérée automatiquement.
      const vide = tbody.querySelector('tr[data-empty-state="true"]');
      if (vide) vide.style.display = total === 0 ? "" : "none";

      // Quand une ligne « aucun résultat » est déjà affichée, le compteur
      // resterait redondant : on le laisse vide.
      compteur.textContent = total
        ? `Affichage de ${debut + 1}–${Math.min(fin, total)} sur ${total} ligne${
            total > 1 ? "s" : ""
          }`
        : vide
        ? ""
        : "Aucun résultat";

      dessinerBoutons(pages);
    }

    function bouton(libelle, page, options) {
      const opts = options || {};
      const li = document.createElement("li");
      li.className =
        "page-item" +
        (opts.actif ? " active" : "") +
        (opts.desactive ? " disabled" : "");
      const a = document.createElement("button");
      a.type = "button";
      a.className = "page-link";
      a.innerHTML = libelle;
      if (!opts.desactive && !opts.actif) {
        a.addEventListener("click", function () {
          pageCourante = page;
          rendre();
          table.scrollIntoView({ block: "nearest", behavior: "smooth" });
        });
      }
      li.appendChild(a);
      return li;
    }

    function dessinerBoutons(pages) {
      pagination.innerHTML = "";
      if (pages <= 1) {
        pagination.parentElement.style.display = "none";
        return;
      }
      pagination.parentElement.style.display = "";

      pagination.appendChild(
        bouton('<i class="fas fa-chevron-left"></i>', pageCourante - 1, {
          desactive: pageCourante === 1,
        })
      );

      // Fenêtre glissante avec points de suspension.
      const numeros = new Set([1, pages, pageCourante]);
      for (let i = pageCourante - 1; i <= pageCourante + 1; i++) {
        if (i > 1 && i < pages) numeros.add(i);
      }
      const liste = Array.from(numeros).sort((a, b) => a - b);
      let precedent = 0;
      liste.forEach((n) => {
        if (n - precedent > 1) {
          const li = document.createElement("li");
          li.className = "page-item disabled";
          li.innerHTML = '<span class="page-link">…</span>';
          pagination.appendChild(li);
        }
        pagination.appendChild(bouton(String(n), n, { actif: n === pageCourante }));
        precedent = n;
      });

      pagination.appendChild(
        bouton('<i class="fas fa-chevron-right"></i>', pageCourante + 1, {
          desactive: pageCourante === pages,
        })
      );
    }

    if (selecteurTaille) {
      selecteurTaille.addEventListener("change", function () {
        pageCourante = 1;
        rendre();
      });
    }

    return {
      table: table,
      rafraichir: function (reinitialiser) {
        if (reinitialiser !== false) pageCourante = 1;
        rendre();
      },
    };
  }

  document.addEventListener("DOMContentLoaded", function () {
    const instances = [];
    document.querySelectorAll("table").forEach(function (table) {
      if (table.dataset.noPagination === "true") return;
      const tbody = table.querySelector("tbody");
      if (!tbody) return;
      const instance = creerPagination(table);
      if (instance) instances.push(instance);
    });

    window.bwPagination = {
      rafraichir: function (table) {
        instances.forEach(function (i) {
          if (!table || i.table === table) i.rafraichir();
        });
      },
    };

    instances.forEach((i) => i.rafraichir());

    // Les scripts de filtrage signalent leurs changements par cet évènement.
    document.addEventListener("bw:filtre", function (e) {
      const cible = e.detail && e.detail.table ? e.detail.table : null;
      window.bwPagination.rafraichir(cible);
    });
  });
})();
