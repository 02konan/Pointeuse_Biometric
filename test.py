from ctypes import *

# Charger la DLL SDK Hikvision
sdk = cdll.LoadLibrary("lib/HCNetSDK.dll")

# Initialiser le SDK
if not sdk.NET_DVR_Init():
    print("❌ Échec de l'initialisation du SDK")
    exit()

class NET_DVR_DEVICEINFO_V30(Structure):
    _fields_ = [
        ("sSerialNumber", c_byte * 48),
        ("byAlarmInPortNum", c_byte),
        ("byAlarmOutPortNum", c_byte),
        ("byDiskNum", c_byte),
        ("byDVRType", c_byte),
        ("byChanNum", c_byte),
        ("byStartChan", c_byte),
        ("byAudioChanNum", c_byte),
        ("byIPChanNum", c_byte),
        ("byZeroChanNum", c_byte),
        ("byMainProto", c_byte),
        ("bySubProto", c_byte),
        ("bySupport", c_byte),
        ("bySupport1", c_byte),
        ("bySupport2", c_byte),
        ("wDevType", c_ushort),
        ("bySupport3", c_byte),
        ("byMultiStreamProto", c_byte),
        ("byStartDChan", c_byte),
        ("byStartDTalkChan", c_byte),
        ("byHighDChanNum", c_byte),
        ("bySupport4", c_byte),
        ("byLanguageType", c_byte),
        ("byVoiceInChanNum", c_byte),
        ("byStartVoiceInChanNo", c_byte),
        ("bySupport5", c_byte),
        ("bySupport6", c_byte),
        ("byMirrorChanNum", c_byte),
        ("wStartMirrorChanNo", c_ushort),
        ("bySupport7", c_byte),
        ("byRes2", c_byte * 2)
    ]

device_info = NET_DVR_DEVICEINFO_V30()
sdk.NET_DVR_Login_V30.restype = c_long

# MODIFIE ICI AVEC TON IP / PORT / UTILISATEUR / MOT DE PASSE
user_id = sdk.NET_DVR_Login_V30(b"192.168.1.252", 8000, b"admin", b"12345678s", byref(device_info))

if user_id < 0:
    print("❌ Connexion échouée. Code erreur :", sdk.NET_DVR_GetLastError())
    sdk.NET_DVR_Cleanup()
    exit()

print("✅ Connecté avec succès !")

# Afficher quelques infos
serial = bytes(device_info.sSerialNumber).split(b'\x00', 1)[0].decode()
print(f"Numéro de série : {serial}")
print(f"Type DVR : {device_info.byDVRType}")
print(f"Nombre de canaux : {device_info.byChanNum}")

# Déconnexion
sdk.NET_DVR_Logout(user_id)
sdk.NET_DVR_Cleanup()
