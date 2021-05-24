CREATE USER 'mt2'@'localhost' IDENTIFIED BY 'mt2';

GRANT SELECT, INSERT, UPDATE, DELETE ON `player`.* TO 'mt2'@'localhost';

GRANT SELECT, INSERT, UPDATE, DELETE ON `log`.* TO 'mt2'@'localhost';

GRANT SELECT, INSERT, UPDATE, DELETE ON `anticheat`.* TO 'mt2'@'localhost';

GRANT SELECT, INSERT, UPDATE, DELETE ON `common`.* TO 'mt2'@'localhost';

GRANT SELECT, INSERT, UPDATE, DELETE ON `account`.* TO 'mt2'@'localhost';

