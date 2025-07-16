function telechargerFichePresence(type, btn) {
    btn.disabled = true;
    const original = btn.innerHTML;
    btn.innerHTML = '<span class="spinner-border spinner-border-sm me-2"></span>Génération en cours...';
    const progress = document.getElementById("progress-bar-fiche");
    progress.style.width = "10%";
    progress.classList.remove("d-none");

    fetch("/api/fiche_presence", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({
        type: type,
        date_debut: document.getElementById("date_debut").value,
        date_fin: document.getElementById("date_fin").value,
      }),
    })
      .then((response) => {
        if (!response.ok) {
          return response.text().then(text => {
            let msg = "Erreur lors de la génération du PDF";
            try {
              const err = JSON.parse(text);
              msg = err.error || msg;
            } catch (e) {}
            throw new Error(msg);
          });
        }
        // Vérification du type de contenu
        const contentType = response.headers.get("Content-Type");
        if (!contentType || !contentType.includes("pdf")) {
            throw new Error("La réponse n'est pas un PDF.");
        }
        progress.style.width = "60%";
        return response.blob();
      })
      .then((blob) => {
        // Si le PDF est vide, ne pas déclencher le téléchargement
        if (blob.size === 0) {
          throw new Error("Le fichier PDF généré est vide.");
        }
        progress.style.width = "100%";
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement("a");
        a.href = url;
        a.download = `fiche_${type}.pdf`;
        document.body.appendChild(a);
        a.click();
        a.remove();
        setTimeout(() => {
          progress.classList.add("d-none");
          progress.style.width = "0%";
          btn.disabled = false;
          btn.innerHTML = original;
        }, 1000);
      })
      .catch((error) => {
        btn.disabled = false;
        btn.innerHTML = original;
        progress.classList.add("d-none");
        progress.style.width = "0%";
        alert(error.message || "Erreur lors de la génération du PDF");
      });
  }