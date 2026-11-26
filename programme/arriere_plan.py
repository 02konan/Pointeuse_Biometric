
import threading
import time
# from programme.attendance import synchronisation_attendance
# from programme.eduflowApi import sync_programme_periodique
from programme.pointeuses import journal, superviseur

PAUSE_APRES_PANNE = 30


def _toujours(nom, boucle, *args):
    def executer():
        while True:
            try:
                boucle(*args)
                journal(f"{nom} : boucle terminée, relance dans {PAUSE_APRES_PANNE} s.")
            except Exception as erreur:
                journal(f"{nom} : arrêt sur erreur ({erreur!r}), relance dans {PAUSE_APRES_PANNE} s.")
            time.sleep(PAUSE_APRES_PANNE)

    threading.Thread(target=executer, name=nom, daemon=True).start()


def demarrer_services():
    superviseur.demarrer()
    # _toujours("synchro-eduflow", sync_programme_periodique, 180)
    # _toujours("presences-programme", synchronisation_attendance, 60)
