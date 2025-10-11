import pymysql
import os

def connexion():
    connection = pymysql.connect(
        host='mysql-divix.alwaysdata.net',
        db='divix_bd',
        user='divix',
        password='Biometricifsm@2025',
        connect_timeout=10
    )
    return connection

