"""Construction de l'application Flask et de ses extensions."""

import os
import secrets
from datetime import timedelta
from pathlib import Path

from flask import Flask
from flask_cors import CORS

from App.routes import api as api_mobile_blueprint


class ApplicationFactory:
    """Creates configured Flask application instances."""

    def __init__(self, configuration=None):
        self._configuration = configuration or {}
        self._project_root = Path(__file__).resolve().parent.parent

    def create_app(self):
        app = Flask(
            __name__,
            static_folder=str(self._project_root / "static"),
            template_folder=str(self._project_root / "template"),
        )
        app.config.update(self._configuration)
        if not app.config.get("SECRET_KEY"):
            app.config["SECRET_KEY"] = (
                os.environ.get("SECRET_KEY") or secrets.token_hex(32)
            )
        app.permanent_session_lifetime = timedelta(minutes=10)
        CORS(app)
        app.register_blueprint(api_mobile_blueprint)
        return app
