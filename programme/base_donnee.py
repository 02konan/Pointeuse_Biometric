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
#         db='biopro_base',
#         user='root',
#         password=''
#     )
#     return connection
# def connexion():
#     connection = pymysql.connect(
#         host='91.234.195.181',
#         db='c2671149c_biopro_base',
#         user='c2671149c_ifsmdev',
#         password='FSYP7AkY'
#     )
#     return connection    