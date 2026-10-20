from database.connection import DatabaseConnectionFactory


class Database:
    """Application database gateway."""

    def __init__(self, connection_factory=None):
        self._connection_factory = connection_factory

    def connect(self):
        factory = self._connection_factory or DatabaseConnectionFactory.from_environment()
        return factory.connect()


_database = Database()


def connexion():
    """Compatibility entry point used by existing repositories and services."""
    return _database.connect()
