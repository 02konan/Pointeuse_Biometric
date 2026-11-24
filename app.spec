# -*- mode: python ; coding: utf-8 -*-
# Compilation de l'application de bureau Divix :
#
#     pyinstaller --noconfirm app.spec
#
# Résultat : dist/Divix/Divix.exe (dossier complet à distribuer).
#
# Configuration : le fichier .env n'est PAS intégré au paquet (il contient le
# mot de passe de la base). S'il existe au moment de la compilation, il est
# copié à côté de l'exécutable, dans dist/Divix/.env, où on peut le modifier
# sans recompiler. database/connection.py le cherche à cet endroit.
#
# PyInstaller ne compile pas d'un système à l'autre : compiler sous Windows
# pour obtenir un .exe Windows.

import os
import shutil

from PyInstaller.utils.hooks import collect_all, collect_submodules

NOM = 'Divix'

# Dans chaque paire (source, destination), la destination est un DOSSIER
# relatif à la racine du paquet.
datas = [
    ('template', 'template'),
    ('static', 'static'),
]
binaries = []
hiddenimports = (
    collect_submodules('programme')
    + collect_submodules('App')
    + collect_submodules('database')
    + ['pymysql', 'openpyxl', 'zk']
)

# Paquets qui chargent des fichiers ou des modules à l'exécution :
# pywebview (moteurs de fenêtre, pythonnet sous Windows), WeasyPrint (feuilles
# de style), reportlab (polices).
for paquet in ('webview', 'weasyprint', 'reportlab'):
    d, b, h = collect_all(paquet)
    datas += d
    binaries += b
    hiddenimports += h


a = Analysis(
    ['app.py'],
    pathex=[],
    binaries=binaries,
    datas=datas,
    hiddenimports=hiddenimports,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    # Outils de développement et bibliothèques lourdes non utilisées.
    excludes=['tkinter', 'pytest', 'IPython', 'matplotlib', 'numpy', 'pandas'],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    [],
    exclude_binaries=True,
    name=NOM,
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=False,
    # Console visible : les erreurs (base injoignable, .env absent…)
    # s'y affichent. Passer à False une fois l'application stable.
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)
coll = COLLECT(
    exe,
    a.binaries,
    a.datas,
    strip=False,
    upx=False,
    upx_exclude=[],
    name=NOM,
)

# Copie du .env à côté de l'exécutable (voir l'en-tête).
if os.path.isfile('.env'):
    shutil.copy2('.env', os.path.join(DISTPATH, NOM, '.env'))
    print(f"[Divix] .env copié dans {os.path.join(DISTPATH, NOM)}")
else:
    print(f"[Divix] Aucun .env : en déposer un dans {os.path.join(DISTPATH, NOM)} avant de lancer.")
