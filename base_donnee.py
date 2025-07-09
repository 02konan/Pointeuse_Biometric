import pymysql

def connexion():
    connection = pymysql.connect(
        host="localhost",
        user="root",
        password="",
        database="bioprodb"
    )
    return connection