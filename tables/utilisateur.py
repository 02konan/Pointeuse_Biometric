from database.db import db
class Utilisateur(db.Model):
    __tablename__ = 'utilisateurs'

    id = db.Column(db.Integer, primary_key=True)
    nom = db.Column(db.String(100), nullable=False)
    email = db.Column(db.String(100), nullable=False, unique=True)
    mot_de_passe = db.Column(db.String(255), nullable=False)
    role_id = db.Column(db.Integer, db.ForeignKey('roles.id'), nullable=False)
    IDSection = db.Column(db.Integer, db.ForeignKey('section.IDSection'), nullable=False)

    # Relations
    role = db.relationship('Role', back_populates='utilisateurs')
    section = db.relationship('Section', back_populates='utilisateurs')

    def __repr__(self):
        return f'<Utilisateur {self.id}: {self.nom} ({self.email})>'