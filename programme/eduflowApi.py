import requests
from programme.base_donnee import connexion
import pymysql
import time

def api_programme():
    url = "https://eduflow.ifsmedu.com/api-pointer.php"
    try:
        response = requests.get(url)
        if response.status_code == 200:
            try:
                data = response.json()
            except Exception as e:
                print(f"[ERREUR] Impossible de parser la réponse JSON : {e}")
                print(f"[DEBUG] Contenu brut : {response.text}")
                return {"error": f"Réponse non JSON : {response.text}"}

            programmes = data.get("results", [])
            if not isinstance(programmes, list):
                print(f"[ERREUR] La clé 'results' n'est pas une liste : {programmes}")
                return {"error": f"Format inattendu : {programmes}"}

            with connexion() as conn:
                with conn.cursor() as curseur:
                    for programme in programmes:
                        if not isinstance(programme, dict):
                            print(f"[ERREUR] Élément inattendu dans la liste : {programme}")
                            continue
                        professeur_id = programme.get('professeur_id')
                        professeur_code = programme.get('professeur_code')
                        professeur_nom = programme.get('professeur_nom')
                        jour = programme.get('jour')
                        heure_arrivee = programme.get('heure_arrivee')
                        heure_depart = programme.get('heure_depart')
                        duree_cours = programme.get('duree_cours')
                        section = programme.get('section')
                        type_edt = programme.get('type_edt')
                        statut = programme.get('statut')
                        curseur.execute("SELECT IDProgramme FROM Programme WHERE professeur_id = %s AND jour = %s", (professeur_id, jour))
                        existe = curseur.fetchone()
                        
                        if existe:
                            identifiant_programme = existe[0]
                            sql = """
                             UPDATE IGNORE Programme
                             
                             SET professeur_code=%s,
                             professeur_nom=%s,
                             jour=%s,
                             heure_arrivee=%s,
                             heure_depart=%s,
                             duree_cours=%s,
                             section=%s,
                             type_edt=%s,
                             status=%s
                             WHERE IDProgramme=%s AND professeur_id=%s AND jour=%s
                            """
                            curseur.execute(sql, (professeur_code, professeur_nom, jour,heure_arrivee, heure_depart, duree_cours,section,type_edt,statut, identifiant_programme, professeur_id, jour))
                            print("Programme mis à jour avec succès.")
                        else:
                            sql = """
                            INSERT IGNORE INTO Programme (professeur_id,professeur_code, professeur_nom, jour,heure_arrivee, heure_depart, duree_cours, section, type_edt, status)
                            VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
                            """
                            curseur.execute(sql, (professeur_id, professeur_code, professeur_nom, jour,heure_arrivee, heure_depart, duree_cours,section,type_edt,statut))
                            print("Nouveau programme inséré avec succès.")
                conn.commit()
            return {"success": True}
        else:
            return {"error": f"Erreur {response.status_code}"}
    except pymysql.MySQLError as e:
        print(f"Erreur MySQL : {e}")
        return {"error": str(e)}
    except Exception as e:
        print(f"Erreur : {e}")
        return {"error": str(e)}

def sync_programme_periodique(period=180):
    while True:
        print("[INFO] Synchronisation des programmes avec Eduflow...")
        result = api_programme()
        print(f"[INFO] Résultat synchronisation : {result}")
        print(f"[INFO] Prochaine synchronisation dans {period} secondes...")
        time.sleep(period)
