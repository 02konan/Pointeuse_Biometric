from database import db

class Empreinte(db.Model):
    __tablename__ = 'empreintes'

    IDEmploye = db.Column(db.Integer, db.ForeignKey('employe.id'), primary_key=True)
    Matricule = db.Column(db.String(100), nullable=False)
    IDEmpreinte = db.Column(db.Integer, nullable=False)
    Empreintes = db.Column(db.String(100), nullable=False)
    IDPointeuse = db.Column(db.Integer, db.ForeignKey('pointeuse.idPointeuse'), nullable=False)

    # Relations
    employe = db.relationship('Employe', back_populates='empreintes')
    pointeuse = db.relationship('Pointeuse', back_populates='empreintes')

    def __repr__(self):
        return f'<Empreinte {self.Matricule}: {self.IDEmpreinte}>'