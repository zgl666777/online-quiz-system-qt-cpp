#include "myserver.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <jsoncpp/json/writer.h>
#include <sstream>

namespace {
const char *envOr(const char *name, const char *fallback)
{
    const char *value = std::getenv(name);
    return value && *value ? value : fallback;
}
}

MyServer::MyServer()
{
    log = spdlog::stdout_color_mt("quiz-server");
    db = new DB(envOr("QUIZ_DB_HOST", "127.0.0.1"), envOr("QUIZ_DB_USER", "quiz"),
                envOr("QUIZ_DB_PASSWORD", "Quiz@12345678"), envOr("QUIZ_DB_NAME", "quiz_system"));
}

MyServer::~MyServer() { delete db; }

void MyServer::connectEvent(TcpSocket *socket)
{
    std::lock_guard<std::mutex> lock(mutex);
    connections.emplace(socket, ConnectionState{});
    log->info("client connected {}:{}", socket->getIp(), socket->getPort());
}

void MyServer::readEvent(TcpSocket *socket)
{
    char data[4096];
    int count;
    std::lock_guard<std::mutex> lock(mutex);
    auto found = connections.find(socket);
    if (found == connections.end()) return;
    while ((count = socket->readData(data, sizeof(data))) > 0) found->second.input.append(data, count);

    auto &state = found->second;
    while (true) {
        if (state.expected == 0) {
            if (state.input.size() < 4) return;
            uint32_t networkSize;
            std::memcpy(&networkSize, state.input.data(), 4);
            state.expected = ntohl(networkSize);
            state.input.erase(0, 4);
            if (state.expected == 0 || state.expected > 1024 * 1024) return;
        }
        if (state.input.size() < state.expected) return;
        const std::string payload = state.input.substr(0, state.expected);
        state.input.erase(0, state.expected);
        state.expected = 0;
        Json::Value request;
        Json::CharReaderBuilder builder;
        std::string errors;
        std::istringstream stream(payload);
        if (Json::parseFromStream(builder, stream, &request, &errors)) handle(socket, request);
        else log->warn("invalid json: {}", errors);
    }
}

void MyServer::handle(TcpSocket *socket, const Json::Value &request)
{
    switch (request["cmd"].asInt()) {
    case REGISTER: registerUser(socket, request); break;
    case LOGIN: loginUser(socket, request); break;
    case SINGLE_GETQUESTION: sendQuestions(socket, SINGLE_GETQUESTION); break;
    case RANK: joinRank(socket); break;
    case ANSWER: answerRank(socket, request); break;
    case RANKRESULT: finishRank(socket, request); break;
    case LEADERBOARD: sendLeaderboard(socket); break;
    case MATCH_HISTORY: sendMatchHistory(socket); break;
    default: break;
    }
}

void MyServer::sendJson(TcpSocket *socket, const Json::Value &json)
{
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    const std::string payload = Json::writeString(builder, json);
    const uint32_t size = htonl(static_cast<uint32_t>(payload.size()));
    socket->writeData(&size, sizeof(size));
    socket->writeData(payload.data(), payload.size());
}

void MyServer::registerUser(TcpSocket *socket, const Json::Value &request)
{
    Json::Value response;
    response["cmd"] = REGISTER;
    const std::string name = request["userName"].asString();
    const std::string password = request["passwd"].asString();
    if (name.empty() || password.size() < 4 || name.size() > 32) response["result"] = ERROR;
    else if (db->userExists(name)) response["result"] = USEREXIST;
    else response["result"] = db->createUser(name, password) ? OK : ERROR;
    log->info("register user={} result={}", name, response["result"].asInt());
    sendJson(socket, response);
}

void MyServer::loginUser(TcpSocket *socket, const Json::Value &request)
{
    Json::Value response;
    response["cmd"] = LOGIN;
    const std::string name = request["userName"].asString();
    int score = 0;
    if (!db->authenticate(name, request["passwd"].asString(), score)) response["result"] = NAMEORPASSWD;
    else if (onlineUsers.count(name)) response["result"] = USERLOGIN;
    else {
        onlineUsers[name] = socket;
        connections[socket].userName = name;
        response["result"] = OK;
        response["userName"] = name;
        response["rank"] = rankName(score);
    }
    sendJson(socket, response);
}

void MyServer::sendQuestions(TcpSocket *socket, int command)
{
    Json::Value response;
    response["cmd"] = command;
    response["question"] = db->randomQuestions(QUESTION_NUM);
    sendJson(socket, response);
}

void MyServer::joinRank(TcpSocket *socket)
{
    if (connections[socket].userName.empty() || matches.count(socket)) return;
    while (!waiting.empty() && !connections.count(waiting.front())) waiting.pop_front();
    if (waiting.empty()) { waiting.push_back(socket); return; }
    TcpSocket *enemy = waiting.front();
    waiting.pop_front();
    matches[socket].enemy = enemy;
    matches[enemy].enemy = socket;
    const Json::Value questions = db->randomQuestions(QUESTION_NUM);
    for (auto pair : {std::make_pair(socket, enemy), std::make_pair(enemy, socket)}) {
        int enemyScore = 0;
        db->userScore(connections[pair.second].userName, enemyScore);
        Json::Value response;
        response["cmd"] = RANK;
        response["question"] = questions;
        response["enemyName"] = connections[pair.second].userName;
        response["enemyRank"] = rankName(enemyScore);
        response["enemyScore"] = 0;
        sendJson(pair.first, response);
    }
}

void MyServer::answerRank(TcpSocket *socket, const Json::Value &request)
{
    auto match = matches.find(socket);
    if (match == matches.end() || !match->second.enemy) return;
    match->second.score = request["score"].asInt();
    match->second.questionId = request["questionId"].asInt();
    Json::Value response;
    response["cmd"] = ANSWER;
    response["enemyscore"] = match->second.score;
    response["enemyQuestionId"] = match->second.questionId;
    sendJson(match->second.enemy, response);
}

void MyServer::finishRank(TcpSocket *socket, const Json::Value &request)
{
    const std::string name = connections[socket].userName;
    int oldScore = 0;
    db->userScore(name, oldScore);
    const auto match = matches.find(socket);
    if (match != matches.end() && match->second.finished) return;
    const int myScore = request["score"].asInt();
    int enemyScore = request["enemyScore"].asInt();
    if (match != matches.end() && match->second.enemy) {
        const auto enemy = matches.find(match->second.enemy);
        if (enemy != matches.end()) enemyScore = enemy->second.score;
    }
    const int delta = myScore > enemyScore ? 20 : (myScore < enemyScore ? -10 : 0);
    const int newScore = std::max(0, oldScore + delta);
    db->updateScore(name, newScore);
    const std::string opponentName = match != matches.end() && match->second.enemy
                                         && connections.count(match->second.enemy)
                                     ? connections[match->second.enemy].userName : request["enemyName"].asString();
    const std::string result = myScore > enemyScore ? "胜利" : myScore < enemyScore ? "失败" : "平局";
    db->addMatchRecord(name, opponentName, myScore, enemyScore, result, delta, false);
    if (match != matches.end()) match->second.finished = true;
    Json::Value response;
    response["cmd"] = RANKRESULT;
    response["newRank"] = rankName(newScore);
    sendJson(socket, response);
}

void MyServer::sendLeaderboard(TcpSocket *socket)
{
    Json::Value response;
    response["cmd"] = LEADERBOARD;
    response["players"] = db->leaderboard(20);
    sendJson(socket, response);
}

void MyServer::sendMatchHistory(TcpSocket *socket)
{
    Json::Value response;
    response["cmd"] = MATCH_HISTORY;
    const auto connection = connections.find(socket);
    response["records"] = connection == connections.end() || connection->second.userName.empty()
                              ? Json::Value(Json::arrayValue)
                              : db->matchHistory(connection->second.userName, 50);
    sendJson(socket, response);
}

std::string MyServer::rankName(int score) const
{
    if (score >= 1000) return "钻石";
    if (score >= 600) return "黄金";
    if (score >= 300) return "白银";
    return "青铜";
}

void MyServer::closeEvent(TcpSocket *socket, short)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto match = matches.find(socket);
    if (match != matches.end()) {
        TcpSocket *enemySocket = match->second.enemy;
        auto enemyMatch = matches.find(enemySocket);
        const bool matchInProgress = !match->second.finished && enemyMatch != matches.end()
                                     && !enemyMatch->second.finished
                                     && connections.count(enemySocket) > 0;
        if (matchInProgress) {
            const std::string enemyName = connections[enemySocket].userName;
            const std::string leavingName = connections.count(socket) ? connections[socket].userName : "";
            int oldScore = 0;
            db->userScore(enemyName, oldScore);
            const int newScore = oldScore + 20;
            db->updateScore(enemyName, newScore);
            db->addMatchRecord(enemyName, leavingName, enemyMatch->second.score,
                               match->second.score, "胜利", 20, true);
            if (!leavingName.empty()) {
                int leavingOldScore = 0;
                db->userScore(leavingName, leavingOldScore);
                const int leavingDelta = -std::min(10, leavingOldScore);
                db->updateScore(leavingName, std::max(0, leavingOldScore - 10));
                db->addMatchRecord(leavingName, enemyName, match->second.score,
                                   enemyMatch->second.score, "失败", leavingDelta, true);
            }

            Json::Value response;
            response["cmd"] = RANKRESULT;
            response["forcedWin"] = true;
            response["message"] = "对手已退出，本局获胜";
            response["newRank"] = rankName(newScore);
            sendJson(enemySocket, response);
            log->info("rank player={} wins because opponent disconnected", enemyName);
        }
        if (enemyMatch != matches.end()) matches.erase(enemyMatch);
        matches.erase(match);
    }

    auto found = connections.find(socket);
    if (found != connections.end()) {
        onlineUsers.erase(found->second.userName);
        connections.erase(found);
    }
    waiting.erase(std::remove(waiting.begin(), waiting.end(), socket), waiting.end());
}
