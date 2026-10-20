"""Configuration et création des connexions MySQL."""

import os
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Mapping

import pymysql
from dotenv import load_dotenv


class DatabaseConfigurationError(RuntimeError):
    """Raised when required database settings are missing or invalid."""


def load_project_environment(dotenv_path=None):
    """Load the project .env without overriding explicitly supplied variables."""
    project_root = Path(__file__).resolve().parent.parent
    return load_dotenv(
        dotenv_path=dotenv_path or project_root / ".env",
        override=False,
    )


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
