from database.db import db

class Enseignant(db.Model):
    _tablename_ = 'enseignants'

    id = db.Column(db.Integer, primary_key=True)
    nom = db.Column(db.String(100), nullable=False)
    matricule = db.Column(db.String(50), unique=True, nullable=False)
    email = db.Column(db.String(100))
    photo = db.Column(db.Text)

    pointages = db.relationship('Pointage', back_populates='enseignant')
    etablissements = db.relationship('EnseignantEtablissement', back_populates='enseignant')