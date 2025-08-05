from database.db import db
class Section(db.Model):
    __tablename__ = 'section'

    IDSection = db.Column(db.Integer, primary_key=True)
    idPointeuse = db.Column(db.Integer, db.ForeignKey('pointeuse.idPointeuse'), nullable=False)
    NomSection = db.Column(db.String(100), nullable=False)

    # Relations
    pointeuse = db.relationship('Pointeuse', back_populates='sections')
    utilisateurs = db.relationship('Utilisateur', back_populates='section', cascade='all, delete-orphan')

    def __repr__(self):
        return f'<Section {self.IDSection}: {self.NomSection}>'