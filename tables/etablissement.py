from database.db import db

class Etablissement(db.Model):
    _tablename_ = 'etablissements'

    id = db.Column(db.Integer, primary_key=True)
    nom = db.Column(db.String(100), nullable=False)
    adresse = db.Column(db.Text)

    enseignants = db.relationship('EnseignantEtablissement', back_populates='etablissement')