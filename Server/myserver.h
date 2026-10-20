#ifndef MYSERVER_H
#define MYSERVER_H

#include "tcpserver.h"
#include "db.h"
#include "common.h"
#include <jsoncpp/json/json.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>

class MyServer : public TcpServer
{
public:
    MyServer();
    ~MyServer();

protected:
    void connectEvent(TcpSocket *socket) override;
    void readEvent(TcpSocket *socket) override;
    void closeEvent(TcpSocket *socket, short events) override;

private:
    struct ConnectionState {
        std::string input;
        uint32_t expected = 0;
        std::string userName;
    };
    struct MatchState {
        TcpSocket *enemy = nullptr;
        int score = 0;
        int questionId = 0;
        bool finished = false;
    };

    void handle(TcpSocket *socket, const Json::Value &request);
    void sendJson(TcpSocket *socket, const Json::Value &json);
    void registerUser(TcpSocket *socket, const Json::Value &request);
    void loginUser(TcpSocket *socket, const Json::Value &request);
    void sendQuestions(TcpSocket *socket, int command);
    void joinRank(TcpSocket *socket);
    void answerRank(TcpSocket *socket, const Json::Value &request);
    void finishRank(TcpSocket *socket, const Json::Value &request);
    void sendLeaderboard(TcpSocket *socket);
    void sendMatchHistory(TcpSocket *socket);
    std::string rankName(int score) const;

    std::shared_ptr<spdlog::logger> log;
    DB *db;
    std::mutex mutex;
    std::unordered_map<TcpSocket *, ConnectionState> connections;
    std::unordered_map<std::string, TcpSocket *> onlineUsers;
    std::unordered_map<TcpSocket *, MatchState> matches;
    std::deque<TcpSocket *> waiting;
};

#endif
