from database.db import db

class EnseignantEtablissement(db.Model):
    _tablename_ = 'enseignant_etablissement'

    enseignant_id = db.Column(db.Integer, db.ForeignKey('enseignants.id'), primary_key=True)
    etablissement_id = db.Column(db.Integer, db.ForeignKey('etablissements.id'), primary_key=True)

    enseignant = db.relationship('Enseignant', back_populates='etablissements')
    etablissement = db.relationship('Etablissement', back_populates='enseignants')