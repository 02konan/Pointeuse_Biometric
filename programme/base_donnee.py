import pymysql
import os

def connexion():
    connection = pymysql.connect(
        host='mysql-biopro.alwaysdata.net',
        db='biopro_db',
        user='biopro',
        password='BioPro205ifsm'
    )
    return connection
    