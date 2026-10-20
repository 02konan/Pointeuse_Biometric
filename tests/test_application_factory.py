import unittest

from app import create_app


class ApplicationFactoryTests(unittest.TestCase):
    def test_applies_injected_configuration(self):
        app = create_app({"SECRET_KEY": "test-secret"})

        self.assertEqual(app.secret_key, "test-secret")
        self.assertEqual(app.permanent_session_lifetime.total_seconds(), 600)

    def test_registers_web_and_mobile_routes(self):
        app = create_app({"SECRET_KEY": "test-secret"})
        endpoints = {rule.endpoint for rule in app.url_map.iter_rules()}

        self.assertIn("login", endpoints)
        self.assertIn("intf_employee", endpoints)
        self.assertIn("dashboard_data", endpoints)
        self.assertIn("api_mobile.login", endpoints)
        self.assertIn("api_mobile.pointages", endpoints)

    def test_login_page_remains_available(self):
        app = create_app({"SECRET_KEY": "test-secret"})

        response = app.test_client().get("/login")

        self.assertEqual(response.status_code, 200)


if __name__ == "__main__":
    unittest.main()
