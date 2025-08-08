import pymysql

def connexion():
    connection = pymysql.connect(
        host="91.216.107.248",
        user="ifsme390267",
        password="FSYP7AkY",
        database="bioprodb"
    )
    return connection