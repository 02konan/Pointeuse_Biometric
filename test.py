from datetime import datetime, time
from gerenerateurPdf import generer_fiche_presence_pdf
from read_data import generer_presence
ficheDePresence =generer_presence("2025-02-2","2025-02-10")
generer_fiche_presence_pdf("fichePresence.pdf", ficheDePresence)
