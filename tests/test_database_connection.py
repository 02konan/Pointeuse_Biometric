import unittest
import os
import tempfile
from pathlib import Path
from unittest.mock import patch

from database.connection import (
    DatabaseConfigurationError,
    DatabaseConnectionFactory,
    DatabaseSettings,
    load_project_environment,
)
from programme.api_mobile import MobileAttendanceRepository
from programme.base_donnee import Database


class DatabaseSettingsTests(unittest.TestCase):
    def test_loads_settings_from_explicit_dotenv_file(self):
        with tempfile.TemporaryDirectory() as directory:
            dotenv_path = Path(directory) / ".env"
            dotenv_path.write_text(
                "DB_HOST=dotenv.example.test\n",
                encoding="utf-8",
            )
            with patch.dict(os.environ, {}, clear=False):
                os.environ.pop("DB_HOST", None)
                loaded = load_project_environment(dotenv_path)
                self.assertTrue(loaded)
                self.assertEqual(os.environ["DB_HOST"], "dotenv.example.test")

    def test_reads_database_settings_from_environment_mapping(self):
        settings = DatabaseSettings.from_environment(
            {
                "DB_HOST": "db.example.test",
                "DB_NAME": "attendance",
                "DB_USER": "app",
                "DB_PASSWORD": "secret",
                "DB_PORT": "3307",
                "DB_CONNECT_TIMEOUT": "15",
            }
        )

        self.assertEqual(settings.host, "db.example.test")
        self.assertEqual(settings.name, "attendance")
        self.assertEqual(settings.user, "app")
        self.assertEqual(settings.password, "secret")
        self.assertEqual(settings.port, 3307)
        self.assertEqual(settings.connect_timeout, 15)

    def test_supports_legacy_mysql_environment_names(self):
        settings = DatabaseSettings.from_environment(
            {
                "MYSQL_HOST": "db.example.test",
                "MYSQL_DATABASE": "attendance",
                "MYSQL_USER": "app",
                "MYSQL_PASSWORD": "secret",
            }
        )

        self.assertEqual(settings.host, "db.example.test")
        self.assertEqual(settings.name, "attendance")

    def test_missing_settings_are_reported(self):
        with self.assertRaisesRegex(DatabaseConfigurationError, "DB_PASSWORD"):
            DatabaseSettings.from_environment({})

    def test_invalid_port_is_reported(self):
        with self.assertRaisesRegex(DatabaseConfigurationError, "DB_PORT"):
            DatabaseSettings.from_environment(
                {
                    "DB_HOST": "localhost",
                    "DB_NAME": "attendance",
                    "DB_USER": "app",
                    "DB_PASSWORD": "secret",
                    "DB_PORT": "invalid",
                }
            )

    def test_factory_passes_settings_to_connector(self):
        calls = []
        settings = DatabaseSettings(
            host="db.example.test",
            name="attendance",
            user="app",
            password="secret",
        )
        factory = DatabaseConnectionFactory(
            settings,
            connector=lambda **kwargs: calls.append(kwargs) or "connection",
        )

        self.assertEqual(factory.connect(), "connection")
        self.assertEqual(
            calls,
            [
                {
                    "host": "db.example.test",
                    "database": "attendance",
                    "user": "app",
                    "password": "secret",
                    "port": 3306,
                    "connect_timeout": 10,
                    "charset": "utf8mb4",
                }
            ],
        )

    def test_database_gateway_accepts_an_injected_factory(self):
        factory = type(
            "FakeFactory",
            (),
            {"connect": lambda self: "connection"},
        )()

        self.assertEqual(Database(factory).connect(), "connection")

    def test_mobile_repository_uses_its_injected_connection_factory(self):
        cursor = _FakeCursor((4, "Alice", "hash", "admin", 1))
        connection = _FakeConnection(cursor)
        repository = MobileAttendanceRepository(
            connection_factory=lambda: connection
        )

        compte = repository.compte_utilisateur("alice")

        self.assertEqual(compte["nom"], "Alice")
        self.assertEqual(cursor.params, ("alice",))


class _FakeCursor:
    def __init__(self, row):
        self.row = row
        self.params = None

    def execute(self, sql, params):
        self.params = params

    def __enter__(self):
        return self

    def __exit__(self, exception_type, exception, traceback):
        return False

    def fetchone(self):
        return self.row


class _FakeConnection:
    def __init__(self, cursor):
        self._cursor = cursor

    def __enter__(self):
        return self

    def __exit__(self, exception_type, exception, traceback):
        return False

    def cursor(self):
        return self._cursor


if __name__ == "__main__":
    unittest.main()
