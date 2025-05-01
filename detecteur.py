from zk import ZK, const
import pymysql

zk = ZK('192.168.1.212', port=4370, timeout=15)

try:
    conn = zk.connect()
    conn.disable_device()

    users = conn.get_users()
    for user in users:
        print('User ID:', user.user_id)
        print('Name:', user.name)
        print('Empreinte:', user.uid)
        

        data_base = pymysql.connect(host="localhost", user="root", password="", database="ifsm_database")
        cursor = data_base.cursor()

        for fid in range(10):
            template = conn.get_user_template(user.uid, fid)
            if template and template.size > 0:
                print(f'  Finger ID: {fid} => Template size: {template.size}')
                
                sql = "INSERT INTO empreintes (user_id, nom, finger_id, template) VALUES (%s, %s, %s, %s)"
                cursor.execute(sql, (user.user_id, user.name, fid, template.template))

        data_base.commit()
        cursor.close()
        data_base.close()

    conn.enable_device()
    conn.disconnect()

except Exception as e:
    print("Erreur :", e)
