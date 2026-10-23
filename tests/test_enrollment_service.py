import unittest
from unittest.mock import Mock
from types import SimpleNamespace

from programme.enrollement import FingerprintEnrollmentService


class FingerprintEnrollmentServiceTests(unittest.TestCase):
    def setUp(self):
        self.service = FingerprintEnrollmentService()
        self.connection = Mock()
        self.connection.get_users.return_value = []
        self.connection.next_uid = 12
        self.connection.set_user.return_value = True
        self.service._open_connection = Mock(
            return_value=("device", self.connection, "192.168.1.10")
        )

    def test_enrols_fingerprint_for_new_employee(self):
        self.connection.enroll_user.return_value = True

        result = self.service.enroler_utilisateur(
            user_id="123", name="Ada", finger_index=0
        )

        self.assertTrue(result)
        self.connection.set_user.assert_called_once_with(
            uid=12, name="Ada", user_id="123"
        )
        self.connection.enroll_user.assert_called_once_with(
            uid=12, temp_id=0, user_id="123"
        )
        self.connection.disconnect.assert_called_once_with()

    def test_assigns_card_to_existing_employee(self):
        self.connection.get_users.return_value = [
            SimpleNamespace(user_id="123", uid=5, card=0)
        ]

        result = self.service.enroler_carte(
            user_id="123", name="Ada", card_number=45678
        )

        self.assertTrue(result)
        self.connection.set_user.assert_called_once_with(
            uid=5, name="Ada", user_id="123", card=45678
        )
        self.connection.disconnect.assert_called_once_with()

    def test_rejects_card_already_assigned_to_another_employee(self):
        self.connection.get_users.return_value = [
            SimpleNamespace(user_id="456", uid=5, card=45678)
        ]

        result = self.service.enroler_carte(
            user_id="123", name="Ada", card_number=45678
        )

        self.assertFalse(result)
        self.connection.set_user.assert_not_called()
        self.connection.disconnect.assert_called_once_with()


if __name__ == "__main__":
    unittest.main()
