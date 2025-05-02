from flask import Flask, render_template, request, redirect, url_for
from flask_cors import CORS
from read_data import read_data_from_db
from database.db import db
import os
# import detecteur

app= Flask(__name__, static_folder='static', template_folder='template')
app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('LIEN_DE_LABASE')
app.config['SQLALCHEMY_BINDS'] = {
    'default': os.getenv('LIEN_DE_LABASE')
}
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db.init_app(app)
with app.app_context():
    db.create_all()
CORS(app)

@app.route('/')
def index():
    data = read_data_from_db()
    if data:
        total_eleves, total_Presents, total_retard, activité_recentes = data
        employee={
            'total_eleves': total_eleves,
            'presents': total_Presents,
            'retard': total_retard,
            'activité_recentes':activité_recentes,
        }
        employees=[employee]
    else:
        employees=[]
    return render_template('index.html', employee_data=employees)


if __name__ =='__main__':
    app.run(debug=True)