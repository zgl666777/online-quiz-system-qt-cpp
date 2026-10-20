#ifndef BRAINSTORM_H
#define BRAINSTORM_H

#include <QDialog>
#include <QTimer>
#include <QJsonArray>
#include "communicate.h"
#include "../common.h"

namespace Ui {
class BrainStorm;
}

class BrainStorm : public QDialog
{
    Q_OBJECT

public:
    explicit BrainStorm(Communicate *com, QJsonObject &json, QWidget *parent = nullptr);
    ~BrainStorm();

private:
    //单人训练从服务器获取题目
    void singleGetQuestion();

    //单人训练设置问题
    void singleSetQuestion();

    //单人训练答题
    void singleAnswerQuestion(int select);

    //设置得分
    void setEnemyScore();
    void setSelfScore();

    //rank设置问题
    void rankSetQuestion();

    //rank答题
    void rankAnswerQuestion(int select);

    //rank结果
    void rankSetResult(QJsonObject &json);
private slots:
    void on_singleButton_clicked();

    void on_single_back_clicked();

    void on_single_start_clicked();

    //单人训练从服务器接收题目
    void receiveSingleQuestion(QJsonObject json);

    void singleTimerOut();

    void on_pushButton_clicked();

    void on_singleSelectButton1_clicked();

    void on_singleSelectButton2_clicked();

    void on_singleSelectButton3_clicked();

    void on_singleSelectButton4_clicked();

    void on_rankButton_clicked();

    //排位接收
    void Rank(QJsonObject json);

    void rankTimerOut();
    void on_rankSelectButton1_clicked();

    void on_rankSelectButton2_clicked();

    void on_rankSelectButton3_clicked();

    void on_rankSelectButton4_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_leaderBoardButton_clicked();

    void leaderBoard(QJsonObject json);
    void matchHistory(QJsonObject json);
private:
    Ui::BrainStorm *ui;

    Communicate *_com;  //复制和服务器进行通信

    QJsonObject _singleQuestion;

    int _currentSingleQuestion; //当前问题下标

    QTimer _singleTimer;  //个人训练赛计时器

    int _singleSec;    //个人训练答题时间

    int _singleScore;  //个人训练得分


    //----------------  rank  ---------------
    QJsonObject _rankQuestion;   //rank题
    QString _enemyName;  //对手的名字
    QString _enemyRank;  //对手的段位
    int _enemyScore;  // 对手的得分

    int _rankSec;  // 答题计时
    int _myScore;  //自己的得分

    QString _userName; //自己的用户名
    QString _userRank;// 自己的得分


    int _currentRankQuestion;  //当前回答问题下标
    int _enemyRankQuestion;  //对手当前回答问题下标

    QTimer _rankTimer;  //排位赛计时器
};

#endif // BRAINSTORM_H
