USE quiz_system;
SET NAMES utf8mb4;
TRUNCATE TABLE questions;

INSERT INTO questions(content,option_a,option_b,option_c,option_d,answer) VALUES
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
