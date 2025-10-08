import pymysql
import os

def connexion():
    connection = pymysql.connect(
        host='mysql-divix.alwaysdata.net',
        db='divix_bd',
        user='divix',
        password='Biometricifsm@2025'
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
#         host='https://www.ifsmunda.com',
#         db='c2671149c_biopro_base',
#         user='c2671149c_ifsmdev',
#         password='FSYP7AkY'
#     )
#     return connection    