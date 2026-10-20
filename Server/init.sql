CREATE DATABASE IF NOT EXISTS quiz_system CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER IF NOT EXISTS 'quiz'@'%' IDENTIFIED BY 'Quiz@12345678';
GRANT ALL PRIVILEGES ON quiz_system.* TO 'quiz'@'%';
FLUSH PRIVILEGES;
USE quiz_system;

CREATE TABLE IF NOT EXISTS users (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(32) NOT NULL UNIQUE,
    password CHAR(64) NOT NULL,
    score INT NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS questions (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    content VARCHAR(500) NOT NULL UNIQUE,
    option_a VARCHAR(200) NOT NULL,
    option_b VARCHAR(200) NOT NULL,
    option_c VARCHAR(200) NOT NULL,
    option_d VARCHAR(200) NOT NULL,
    answer TINYINT UNSIGNED NOT NULL
);

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

INSERT IGNORE INTO questions(content,option_a,option_b,option_c,option_d,answer) VALUES
('在 C++ 中，用于声明常量的关键字是？','const','mutable','volatile','static',1),
('Qt 中用于创建 TCP 客户端套接字的类是？','QUdpSocket','QTcpSocket','QNetworkReply','QFile',2),
('本在线答题系统服务端默认监听的端口是？','80','443','8080','9999',4),
('SQL 中用于对查询结果进行排序的子句是？','GROUP BY','ORDER BY','HAVING','LIMIT',2),
('JSON 的中文含义是？','Java 源对象网络','JavaScript 对象表示法','联合系统对象名称','Java 序列化输出节点',2),
('使用 CMake 配置当前项目的正确命令是？','cmake -S . -B build','make install','qmake clean','gcc configure',1),
('能够提供可靠、有序数据传输的网络协议是？','UDP','ICMP','TCP','ARP',3),
('QTcpSocket 收到可读数据时会发出哪个信号？','connected','readyRead','bytesWritten','hostFound',2),
('C++ 中适合进行键值查找的容器是？','std::vector','std::queue','std::unordered_map','std::stack',3),
('Linux 中常用于调试 C++ 程序的工具是？','gdb','tar','curl','sed',1);
