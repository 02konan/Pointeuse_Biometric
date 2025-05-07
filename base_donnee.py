import pymysql

def connexion():
    connection = pymysql.connect(
        host="localhost",
        user="root",
        password="",
        database="ifsm_database"
    )
    return connection