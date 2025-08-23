import pymysql
import os

def connexion():
    connection = pymysql.connect(
        host='mysql-biopro.alwaysdata.net',
        db='biopro_base',
        user='biopro',
        password='BioPro205ifsm'
    )
    return connection
# def connexion():
#     connection = pymysql.connect(
#         host='localhost',
#         db='bioprodb',
#         user='root',
#         password=''
#     )
#     return connection
    