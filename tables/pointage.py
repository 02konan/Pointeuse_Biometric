from database.db import db

class Pointage(db.Model):
    _tablename_ = 'pointages'

    id = db.Column(db.Integer, primary_key=True)
    enseignant_id = db.Column(db.Integer, db.ForeignKey('enseignants.id'), nullable=False)
    type_pointage = db.Column(db.String(10), nullable=False)  # 'arrivee' ou 'depart'
    date_heure = db.Column(db.DateTime, server_default=db.func.now())

    enseignant = db.relationship('Enseignant', back_populates='pointages')