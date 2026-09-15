-- ---------------------------------------------------------------------------
-- Tables nécessaires à l'application mobile (notifications push).
-- À exécuter une fois sur la base divix_bd avant d'utiliser
--   GET  /api/notifications
--   PUT  /api/notifications/<id>/read
--   POST /api/me/device-token
-- Tant que ces tables n'existent pas, ces routes répondent 503 avec un
-- message explicite ; le reste de l'API mobile fonctionne normalement.
-- ---------------------------------------------------------------------------

CREATE TABLE IF NOT EXISTS notifications_mobile (
    id            INT AUTO_INCREMENT PRIMARY KEY,
    matricule     VARCHAR(50)  NOT NULL,
    titre         VARCHAR(150) NOT NULL,
    message       TEXT         NOT NULL,
    type          VARCHAR(30)  NOT NULL DEFAULT 'info',
    lue           TINYINT(1)   NOT NULL DEFAULT 0,
    date_creation DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_notifications_matricule (matricule, date_creation)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS appareils_mobiles (
    id         INT AUTO_INCREMENT PRIMARY KEY,
    matricule  VARCHAR(50)  NOT NULL,
    jeton_fcm  VARCHAR(255) NOT NULL,
    plateforme VARCHAR(20)  NOT NULL DEFAULT 'inconnue',
    date_maj   DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    -- Un même téléphone ne doit être enregistré qu'une fois : la
    -- réinscription met simplement à jour la ligne existante.
    UNIQUE KEY uk_jeton_fcm (jeton_fcm),
    INDEX idx_appareils_matricule (matricule)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
