from database.db import db
class Pointeuse(db.Model):
    __tablename__ = 'pointeuse'

    idPointeuse = db.Column(db.Integer, primary_key=True)
    NomPointeuse = db.Column(db.String(100), nullable=False)
    Model = db.Column(db.String(100), nullable=False)
    Emplacement = db.Column(db.String(100), nullable=False)
    AdresseIP = db.Column(db.String(100), nullable=False)
    Serie = db.Column(db.String(100), nullable=False)
    Type = db.Column(db.String(100), nullable=False)

    # Relations
    empreintes = db.relationship('Empreinte', back_populates='pointeuse', cascade='all, delete-orphan')
    pointages = db.relationship('Pointage', back_populates='pointeuse', cascade='all, delete-orphan')
    sections = db.relationship('Section', back_populates='pointeuse', cascade='all, delete-orphan')

    def __repr__(self):
        return f'<Pointeuse {self.NomPointeuse}: {self.Model}>'
