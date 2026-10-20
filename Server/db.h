#ifndef DB_H
#define DB_H

#include <jsoncpp/json/json.h>
#include <mysql/mysql.h>
#include <mutex>
#include <string>

class DB
{
public:
    DB(const char *host, const char *userName, const char *password, const char *dbName);
    ~DB();

    bool userExists(const std::string &name);
    bool createUser(const std::string &name, const std::string &password);
    bool authenticate(const std::string &name, const std::string &password, int &score);
    bool userScore(const std::string &name, int &score);
    bool updateScore(const std::string &name, int score);
    Json::Value randomQuestions(int count);
    Json::Value leaderboard(int count);
    bool addMatchRecord(const std::string &userName, const std::string &opponentName,
                        int myScore, int opponentScore, const std::string &result,
                        int scoreDelta, bool forcedEnd);
    Json::Value matchHistory(const std::string &userName, int count);

private:
    std::string escape(const std::string &value);
    bool execute(const std::string &sql);
    MYSQL_RES *query(const std::string &sql);

    std::mutex mutex;
    MYSQL *mysql;
};

#endif
