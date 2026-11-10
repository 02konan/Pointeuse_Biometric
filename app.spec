# -*- mode: python ; coding: utf-8 -*-
# Dans `datas`, le second élément est le DOSSIER de destination : ('.env', '.')
# copie le fichier .env à la racine du paquet. ('.env', '.env') créait un
# dossier « .env » contenant le fichier, et la configuration n'était pas lue.
# Un .env posé à côté de app.exe est prioritaire (voir database/connection.py).
from PyInstaller.utils.hooks import collect_all

datas = [('template', 'template'), ('static', 'static'), ('.env', '.'), ('programme', 'programme'),]
binaries = []
hiddenimports = []
tmp_ret = collect_all('webview')
datas += tmp_ret[0]; binaries += tmp_ret[1]; hiddenimports += tmp_ret[2]


a = Analysis(
    ['app.py'],
    pathex=[],
    binaries=binaries,
    datas=datas,
    hiddenimports=hiddenimports,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    [],
    exclude_binaries=True,
    name='app',
    debug=True,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
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
    upx=True,
    upx_exclude=[],
    name='app',
)
