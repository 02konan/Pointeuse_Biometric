from zk import ZK, const
import pymysql

zk = ZK('192.168.1.212', port=4370, timeout=15)

try:
    conn = zk.connect()
    conn.disable_device()

    users = conn.get_users()
    for user in users:
        

        data_base = pymysql.connect(host="localhost", user="root", password="", database="ifsm_database")
        cursor = data_base.cursor()

        for fid in range(10):
            template = conn.get_user_template(user.uid, fid)
            if template and template.size > 0:
                print(f'  Finger ID: {fid} => Template size: {template.size}')
                
                recup = "SELECT user_id FROM empreintes"
                cursor.execute(recup)
                result = cursor.fetchall()

                user_ids = [row[0] for row in result]
                if user.user_id in user_ids:
                 print("L'empreinte existe déjà")
                else:
                  sql = "INSERT INTO empreintes (user_id, nom, finger_id, template) VALUES (%s, %s, %s, %s)"
                  cursor.execute(sql, (user.user_id, user.name, fid, template.template))

        data_base.commit()
        cursor.close()
        data_base.close()

    conn.enable_device()
    conn.disconnect()

except Exception as e:
    print("Erreur :", e)