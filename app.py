from flask import Flask, render_template, request, redirect, url_for
from flask_cors import CORS
from read_data import read_data_from_db
# import detecteur

app= Flask(__name__, static_folder='static', template_folder='template')
CORS(app)

@app.route('/')
def index():
    # Appel de la fonction pour lire les données de la base de données
    data = read_data_from_db()
    list_donnee = data
    employees = []  
    if list_donnee:
        for list_donnes in list_donnee:
            employee = {
                'id': list_donnes[0],
                'user_id': list_donnes[1],
                'nom': list_donnes[2],
                'finger_id': list_donnes[3],
                'template': list_donnes[4]
            }
            employees.append(employee)
    return render_template('index.html', employee_data=employees)


if __name__ =='__main__':
    app.run(debug=True)