USE quiz_system;
CREATE TABLE IF NOT EXISTS match_history (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    user_name VARCHAR(32) NOT NULL,
    opponent_name VARCHAR(32) NOT NULL,
    my_score INT NOT NULL,
    opponent_score INT NOT NULL,
    result VARCHAR(8) NOT NULL,
    score_delta INT NOT NULL,
    forced_end BOOLEAN NOT NULL DEFAULT FALSE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_match_history_user_time(user_name, created_at)
);
