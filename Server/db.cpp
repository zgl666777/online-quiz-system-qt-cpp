#include "db.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

DB::DB(const char *host, const char *userName, const char *password, const char *dbName)
{
    mysql = mysql_init(nullptr);
    if (!mysql || !mysql_real_connect(mysql, host, userName, password, dbName, 0, nullptr, 0))
        throw std::runtime_error(mysql ? mysql_error(mysql) : "mysql_init failed");
    mysql_set_character_set(mysql, "utf8mb4");
}

DB::~DB() { if (mysql) mysql_close(mysql); }

std::string DB::escape(const std::string &value)
{
    std::string result(value.size() * 2 + 1, '\0');
    const auto length = mysql_real_escape_string(mysql, result.data(), value.data(), value.size());
    result.resize(length);
    return result;
}

bool DB::execute(const std::string &sql)
{
    if (mysql_query(mysql, sql.c_str()) != 0) {
        spdlog::error("mysql query failed: {}", mysql_error(mysql));
        return false;
    }
    return true;
}

MYSQL_RES *DB::query(const std::string &sql)
{
    if (!execute(sql)) return nullptr;
    return mysql_store_result(mysql);
}

bool DB::userExists(const std::string &name)
{
    std::lock_guard<std::mutex> lock(mutex);
    MYSQL_RES *result = query("SELECT 1 FROM users WHERE name='" + escape(name) + "' LIMIT 1");
    if (!result) return false;
    const bool exists = mysql_num_rows(result) > 0;
    mysql_free_result(result);
    return exists;
}

bool DB::createUser(const std::string &name, const std::string &password)
{
    std::lock_guard<std::mutex> lock(mutex);
    return execute("INSERT INTO users(name,password,score) VALUES('" + escape(name) + "',SHA2('" +
                   escape(password) + "',256),0)");
}

bool DB::authenticate(const std::string &name, const std::string &password, int &score)
{
    std::lock_guard<std::mutex> lock(mutex);
    MYSQL_RES *result = query("SELECT score FROM users WHERE name='" + escape(name) +
                              "' AND password=SHA2('" + escape(password) + "',256) LIMIT 1");
    if (!result) return false;
    MYSQL_ROW row = mysql_fetch_row(result);
    const bool valid = row != nullptr;
    if (valid) score = std::stoi(row[0]);
    mysql_free_result(result);
    return valid;
}

bool DB::userScore(const std::string &name, int &score)
{
    std::lock_guard<std::mutex> lock(mutex);
    MYSQL_RES *result = query("SELECT score FROM users WHERE name='" + escape(name) + "' LIMIT 1");
    if (!result) return false;
    MYSQL_ROW row = mysql_fetch_row(result);
    const bool found = row != nullptr;
    if (found) score = std::stoi(row[0]);
    mysql_free_result(result);
    return found;
}

bool DB::updateScore(const std::string &name, int score)
{
    std::lock_guard<std::mutex> lock(mutex);
    return execute("UPDATE users SET score=" + std::to_string(score) + " WHERE name='" + escape(name) + "'");
}

Json::Value DB::randomQuestions(int count)
{
    std::lock_guard<std::mutex> lock(mutex);
    Json::Value questions;
    MYSQL_RES *result = query("SELECT content,option_a,option_b,option_c,option_d,answer FROM questions ORDER BY RAND() LIMIT " + std::to_string(count));
    if (!result) return questions;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        questions["question"].append(row[0]);
        questions["selection1"].append(row[1]);
        questions["selection2"].append(row[2]);
        questions["selection3"].append(row[3]);
        questions["selection4"].append(row[4]);
        questions["answer"].append(row[5]);
    }
    mysql_free_result(result);
    return questions;
}

Json::Value DB::leaderboard(int count)
{
    std::lock_guard<std::mutex> lock(mutex);
    Json::Value players(Json::arrayValue);
    MYSQL_RES *result = query("SELECT name,score FROM users ORDER BY score DESC,name LIMIT " + std::to_string(count));
    if (!result) return players;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        Json::Value player;
        const int score = std::stoi(row[1]);
        player["name"] = row[0];
        player["score"] = score;
        player["rankName"] = score >= 1000 ? "钻石" : score >= 600 ? "黄金" : score >= 300 ? "白银" : "青铜";
        players.append(player);
    }
    mysql_free_result(result);
    return players;
}

bool DB::addMatchRecord(const std::string &userName, const std::string &opponentName,
                        int myScore, int opponentScore, const std::string &result,
                        int scoreDelta, bool forcedEnd)
{
    std::lock_guard<std::mutex> lock(mutex);
    return execute("INSERT INTO match_history(user_name,opponent_name,my_score,opponent_score,result,score_delta,forced_end) VALUES('"
                   + escape(userName) + "','" + escape(opponentName) + "',"
                   + std::to_string(myScore) + "," + std::to_string(opponentScore) + ",'"
                   + escape(result) + "'," + std::to_string(scoreDelta) + ","
                   + (forcedEnd ? "1" : "0") + ")");
}

Json::Value DB::matchHistory(const std::string &userName, int count)
{
    std::lock_guard<std::mutex> lock(mutex);
    Json::Value records(Json::arrayValue);
    MYSQL_RES *result = query("SELECT opponent_name,my_score,opponent_score,result,score_delta,forced_end,"
                              "DATE_FORMAT(created_at,'%Y-%m-%d %H:%i:%s') FROM match_history WHERE user_name='"
                              + escape(userName) + "' ORDER BY created_at DESC,id DESC LIMIT "
                              + std::to_string(count));
    if (!result) return records;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        Json::Value record;
        record["opponentName"] = row[0];
        record["myScore"] = std::stoi(row[1]);
        record["opponentScore"] = std::stoi(row[2]);
        record["result"] = row[3];
        record["scoreDelta"] = std::stoi(row[4]);
        record["forcedEnd"] = std::stoi(row[5]) != 0;
        record["createdAt"] = row[6];
        records.append(record);
    }
    mysql_free_result(result);
    return records;
}
