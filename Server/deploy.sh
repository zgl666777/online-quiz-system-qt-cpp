#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
sudo mysql < "$ROOT_DIR/Server/init.sql"
sudo mysql <<'SQL'
CREATE USER IF NOT EXISTS 'quiz'@'localhost' IDENTIFIED WITH caching_sha2_password BY 'Quiz@12345678';
ALTER USER 'quiz'@'localhost' IDENTIFIED WITH caching_sha2_password BY 'Quiz@12345678';
CREATE USER IF NOT EXISTS 'quiz'@'%' IDENTIFIED WITH caching_sha2_password BY 'Quiz@12345678';
ALTER USER 'quiz'@'%' IDENTIFIED WITH caching_sha2_password BY 'Quiz@12345678';
GRANT ALL PRIVILEGES ON quiz_system.* TO 'quiz'@'localhost';
GRANT ALL PRIVILEGES ON quiz_system.* TO 'quiz'@'%';
FLUSH PRIVILEGES;
SQL

MYSQL_PWD='Quiz@12345678' mysql -uquiz -h127.0.0.1 quiz_system -e 'SELECT COUNT(*) AS questions FROM questions;'
sudo cp "$ROOT_DIR/Server/quiz-server.service" /etc/systemd/system/quiz-server.service
sudo systemctl daemon-reload
sudo systemctl reset-failed quiz-server || true
sudo systemctl enable --now quiz-server
sudo systemctl --no-pager --full status quiz-server
