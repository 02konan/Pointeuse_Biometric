"""Configuration et création des connexions MySQL."""

import os
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Mapping

import pymysql
from dotenv import load_dotenv


class DatabaseConfigurationError(RuntimeError):
    """Raised when required database settings are missing or invalid."""


def env_candidates():
    """Emplacements possibles du fichier .env, du plus prioritaire au moins.

    Une fois compilée avec PyInstaller, l'application ne tourne plus depuis le
    dossier du projet : on cherche d'abord un .env posé à côté de l'exécutable
    (modifiable sans recompiler), puis celui embarqué dans le paquet.
    """
    if getattr(sys, "frozen", False):
        yield Path(sys.executable).resolve().parent / ".env"
        yield Path(getattr(sys, "_MEIPASS", Path(sys.executable).parent)) / ".env"
    yield Path(__file__).resolve().parent.parent / ".env"


def load_project_environment(dotenv_path=None):
    """Load the project .env without overriding explicitly supplied variables."""
    candidates = [Path(dotenv_path)] if dotenv_path else env_candidates()
    for candidate in candidates:
        if candidate.is_file():
            return load_dotenv(dotenv_path=candidate, override=False)
    print("[AVERTISSEMENT] Aucun fichier .env trouvé (cherché : "
          + ", ".join(str(c) for c in env_candidates()) + ")")
    return False


load_project_environment()


@dataclass(frozen=True)
class DatabaseSettings:
    host: str
    name: str
    user: str
    password: str
    port: int = 3306
    connect_timeout: int = 10
    charset: str = "utf8mb4"

    @classmethod
    def from_environment(
        cls, environ: Mapping[str, str] | None = None
    ) -> "DatabaseSettings":
        values = os.environ if environ is None else environ

        def setting(primary: str, legacy: str | None = None) -> str:
            value = values.get(primary)
            if not value and legacy:
                value = values.get(legacy)
            return (value or "").strip()

        host = setting("DB_HOST", "MYSQL_HOST")
        name = setting("DB_NAME", "MYSQL_DATABASE")
        user = setting("DB_USER", "MYSQL_USER")
        password = setting("DB_PASSWORD", "MYSQL_PASSWORD")
        missing = [
            variable
            for variable, value in (
                ("DB_HOST", host),
                ("DB_NAME", name),
                ("DB_USER", user),
                ("DB_PASSWORD", password),
            )
            if not value
        ]
        if missing:
            raise DatabaseConfigurationError(
                "Configuration MySQL manquante : "
                + ", ".join(missing)
                + ". Définissez ces variables d'environnement."
            )

        try:
            port = int(setting("DB_PORT", "MYSQL_PORT") or "3306")
            connect_timeout = int(setting("DB_CONNECT_TIMEOUT") or "10")
        except ValueError as error:
            raise DatabaseConfigurationError(
                "DB_PORT et DB_CONNECT_TIMEOUT doivent être des entiers."
            ) from error

        if not 1 <= port <= 65535:
            raise DatabaseConfigurationError("DB_PORT doit être compris entre 1 et 65535.")
        if connect_timeout <= 0:
            raise DatabaseConfigurationError("DB_CONNECT_TIMEOUT doit être positif.")

        return cls(
            host=host,
            name=name,
            user=user,
            password=password,
            port=port,
            connect_timeout=connect_timeout,
        )
class DatabaseConnectionFactory:
    """Builds PyMySQL connections from injectable settings."""

    def __init__(
        self,
        settings: DatabaseSettings,
        connector: Callable[..., object] = pymysql.connect,
    ) -> None:
        self._settings = settings
        self._connector = connector

    @classmethod
    def from_environment(
        cls,
        environ: Mapping[str, str] | None = None,
        connector: Callable[..., object] = pymysql.connect,
    ) -> "DatabaseConnectionFactory":
        return cls(DatabaseSettings.from_environment(environ), connector)

    def connect(self):
        return self._connector(
            host=self._settings.host,
            database=self._settings.name,
            user=self._settings.user,
            password=self._settings.password,
            port=self._settings.port,
            connect_timeout=self._settings.connect_timeout,
            charset=self._settings.charset,
        )
