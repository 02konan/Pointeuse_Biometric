from database.db import db
class Programme(db.Model):
    __tablename__ = 'programme'

    IDProgramme = db.Column(db.Integer, primary_key=True)
    professeur_id = db.Column(db.Integer)
    professeur_nom = db.Column(db.String(100))
    jour = db.Column(db.String(20))
    heure_arrivee = db.Column(db.Time)
    heure_depart = db.Column(db.Time)
    duree_cours = db.Column(db.Integer)

    # Relations
    pointage_programmes = db.relationship('PointageProgramme', back_populates='programme', cascade='all, delete-orphan')

    def __repr__(self):
        return f'<Programme {self.IDProgramme}: {self.professeur_nom} - {self.jour}>'