import pymysql
import os

def connexion():
    return pymysql.connect(
        host=os.environ['DB_HOST'], 
        port=3306,
        user=os.environ['DB_USER'],
        password=os.environ['DB_PASS'],
        database=os.environ['DB_NAME']
    )