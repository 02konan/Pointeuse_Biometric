from database.db import db
class Pointage(db.Model):
    __tablename__ = 'pointages'

    id = db.Column(db.Integer, primary_key=True)
    IDEmploye = db.Column(db.Integer, db.ForeignKey('employe.id'), nullable=False)
    date_pointage = db.Column(db.DateTime, nullable=False)
    IDPointeuse = db.Column(db.Integer, db.ForeignKey('pointeuse.idPointeuse'), nullable=False)
    jour_pointage = db.Column(db.String(50), nullable=False)

    # Relations
    employe = db.relationship('Employe', back_populates='pointages')
    pointeuse = db.relationship('Pointeuse', back_populates='pointages')
    pointage_programmes = db.relationship('PointageProgramme', back_populates='pointage', cascade='all, delete-orphan')

    def __repr__(self):
        return f'<Pointage {self.id}: Employé {self.IDEmploye} le {self.date_pointage}>'