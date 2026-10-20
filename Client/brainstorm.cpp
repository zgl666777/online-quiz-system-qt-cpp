#include "brainstorm.h"
#include "ui_brainstorm.h"
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

BrainStorm::BrainStorm(Communicate *com, QJsonObject &json, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BrainStorm)
{
    ui->setupUi(this);
    _com = com;

    _userName = json["userName"].toString();
    _userRank = json["rank"].toString();


    connect(&_singleTimer, SIGNAL(timeout()), this, SLOT(singleTimerOut()));
    connect(&_rankTimer, SIGNAL(timeout()), this, SLOT(rankTimerOut()));


    connect(_com, SIGNAL(receiveSingleQuestion(QJsonObject)), this, SLOT(receiveSingleQuestion(QJsonObject)));

    connect(_com, SIGNAL(Rank(QJsonObject)), this, SLOT(Rank(QJsonObject)));

    connect(_com, SIGNAL(leaderBoard(QJsonObject)), this, SLOT(leaderBoard(QJsonObject)));
    connect(_com, &Communicate::matchHistory, this, &BrainStorm::matchHistory);

    auto *historyButton = new QPushButton("历史战绩", ui->mainMenu);
    historyButton->setFixedSize(125, 50);
    ui->gridLayout->removeItem(ui->verticalSpacer_10);
    ui->gridLayout->addWidget(historyButton, 4, 1);
    ui->gridLayout->addItem(ui->verticalSpacer_10, 5, 1);
    connect(historyButton, &QPushButton::clicked, this, [this]() {
        QJsonObject request;
        request["cmd"] = MATCH_HISTORY;
        _com->writeData(request);
    });

}

BrainStorm::~BrainStorm()
{
    delete ui;
}

//个人训练计时器
void BrainStorm::singleTimerOut()
{
    --_singleSec;
    if(_singleSec == 0)
    {
        ui->singleResult->setText("回答错误");
        QString str = QString("本次得分：%1").arg(_singleScore);
        ui->singleScore->setText(str);
        _singleTimer.stop();
        ui->stackedWidget->setCurrentWidget(ui->signle_socre);
    }
    ui->lcdNumber->display(_singleSec);
}

//个人训练题目获取
void BrainStorm::singleGetQuestion()
{
    QJsonObject json;
    json["cmd"] = SINGLE_GETQUESTION;

    _com->writeData(json);
}

//个人训练设置题目
void BrainStorm::singleSetQuestion()
{
    //题目结束
    if(_currentSingleQuestion == QUESTION_NUM)
    {
        ui->singleResult->setText("恭喜全部答对！");
        QString str = QString("本次得分：%1").arg(_singleScore);
        ui->singleScore->setText(str);
        ui->stackedWidget->setCurrentWidget(ui->signle_socre);
        _singleTimer.stop();
        return;
    }

    ui->signleQuestion->setText(_singleQuestion["question"].toArray().at(_currentSingleQuestion).toString());

    ui->singleSelectButton1->setText(_singleQuestion["selection1"].toArray().at(_currentSingleQuestion).toString());
    ui->singleSelectButton2->setText(_singleQuestion["selection2"].toArray().at(_currentSingleQuestion).toString());
    ui->singleSelectButton3->setText(_singleQuestion["selection3"].toArray().at(_currentSingleQuestion).toString());
    ui->singleSelectButton4->setText(_singleQuestion["selection4"].toArray().at(_currentSingleQuestion).toString());

}

//单人训练答题
void BrainStorm::singleAnswerQuestion(int select)
{
    //回答正确
    if(select == _singleQuestion["answer"].toArray().at(_currentSingleQuestion).toString().toInt())
    {
        _singleScore += 20;
        _currentSingleQuestion++;
        singleSetQuestion();
        _singleSec = 10;
        _singleTimer.stop();
        ui->lcdNumber->display(_singleSec);
        _singleTimer.start(1000);

    }
    //回答错误
    else
    {
        ui->singleResult->setText("回答错误");
        QString str = QString("本次得分：%1").arg(_singleScore);
        ui->singleScore->setText(str);
        ui->stackedWidget->setCurrentWidget(ui->signle_socre);
        _singleTimer.stop();
    }
    //题目结束
    if(_currentSingleQuestion == QUESTION_NUM)
    {
        ui->singleResult->setText("恭喜全部答对！");
        QString str = QString("本次得分：%1").arg(_singleScore);
        ui->singleScore->setText(str);
        ui->stackedWidget->setCurrentWidget(ui->signle_socre);
        _singleTimer.stop();
    }
}

//个人训练接收题目
void BrainStorm::receiveSingleQuestion(QJsonObject json)
{
    _singleQuestion = json;

    _singleSec = 10;
    _currentSingleQuestion = 0;
    _singleScore = 0;
    ui->lcdNumber->display(_singleSec);
    singleSetQuestion();

    //开启定时器
    _singleTimer.start(1000);


    ui->stackedWidget->setCurrentWidget(ui->single_running);

}

//进入个人训练
void BrainStorm::on_singleButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->single_menu);

}

//返回
void BrainStorm::on_single_back_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->mainMenu);

}

//开始答题
void BrainStorm::on_single_start_clicked()
{
    singleGetQuestion();

}

//得分界面返回按键
void BrainStorm::on_pushButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->mainMenu);
}

//选项按键
void BrainStorm::on_singleSelectButton1_clicked()
{
    singleAnswerQuestion(1);
}

void BrainStorm::on_singleSelectButton2_clicked()
{
    singleAnswerQuestion(2);
}

void BrainStorm::on_singleSelectButton3_clicked()
{
    singleAnswerQuestion(3);
}

void BrainStorm::on_singleSelectButton4_clicked()
{
    singleAnswerQuestion(4);
}


//---------------------  rank  ---------------------
//排位赛按键
void BrainStorm::on_rankButton_clicked()
{
    QJsonObject json;
    json["cmd"] = RANK;

    _com->writeData(json);

    //跳到等待页面
    ui->stackedWidget->setCurrentWidget(ui->rank_wait);
}

//排行榜按键
void BrainStorm::on_leaderBoardButton_clicked()
{
    QJsonObject json;
    json["cmd"] = LEADERBOARD;

    _com->writeData(json);
}

//接收排行榜
void BrainStorm::leaderBoard(QJsonObject json)
{
    const QJsonArray players = json["players"].toArray();
    QString html = R"(
        <style>
            body { font-family: "Microsoft YaHei UI"; color: #202124; }
            table { width: 100%; border-collapse: collapse; }
            th { padding: 10px 8px; color: #ffffff; background: #356859; text-align: left; }
            td { padding: 10px 8px; border-bottom: 1px solid #dfe5e2; }
            tr:nth-child(even) { background: #f4f7f6; }
            .rank { width: 16%; text-align: center; font-weight: 600; }
            .name { width: 40%; }
            .score { width: 22%; text-align: right; font-weight: 600; }
            .tier { width: 22%; text-align: center; }
            .top1 { color: #b7791f; font-size: 17px; }
            .top2 { color: #667085; font-size: 16px; }
            .top3 { color: #9a5b35; font-size: 16px; }
            .empty { padding: 28px; color: #667085; text-align: center; }
        </style>
        <table>
            <thead><tr><th class="rank">排名</th><th class="name">用户</th><th class="score">积分</th><th class="tier">段位</th></tr></thead>
            <tbody>
    )";

    if (players.isEmpty()) {
        html += "<tr><td class=\"empty\" colspan=\"4\">暂无排行数据</td></tr>";
    }
    for(int i = 0; i < players.size(); i++)
    {
        const QJsonObject player = players[i].toObject();
        const QString rankClass = i == 0 ? "top1" : i == 1 ? "top2" : i == 2 ? "top3" : "";
        html += QString("<tr><td class=\"rank %1\">%2</td><td class=\"name\">%3</td>"
                        "<td class=\"score\">%4</td><td class=\"tier\">%5</td></tr>")
                    .arg(rankClass)
                    .arg(i + 1)
                    .arg(player["name"].toString().toHtmlEscaped())
                    .arg(player["score"].toInt())
                    .arg(player["rankName"].toString().toHtmlEscaped());
    }
    html += "</tbody></table>";
    ui->leaderBoardText->setReadOnly(true);
    ui->leaderBoardText->setHtml(html);
    ui->stackedWidget->setCurrentWidget(ui->rank_board);
}

void BrainStorm::matchHistory(QJsonObject json)
{
    const QJsonArray records = json["records"].toArray();
    QString html = R"(
        <style>
            body { font-family: "Microsoft YaHei UI"; color: #202124; }
            table { width: 100%; border-collapse: collapse; }
            th { padding: 9px 7px; color: white; background: #356859; text-align: center; }
            td { padding: 9px 7px; border-bottom: 1px solid #dfe5e2; text-align: center; }
            tr:nth-child(even) { background: #f4f7f6; }
            .win { color: #16803c; font-weight: 600; }
            .lose { color: #c53030; font-weight: 600; }
            .draw { color: #667085; font-weight: 600; }
            .positive { color: #16803c; }
            .negative { color: #c53030; }
            .empty { padding: 30px; color: #667085; }
        </style>
        <table><thead><tr><th>时间</th><th>对手</th><th>比分</th><th>结果</th><th>积分变化</th></tr></thead><tbody>
    )";
    if (records.isEmpty()) {
        html += "<tr><td class=\"empty\" colspan=\"5\">暂无排位对战记录</td></tr>";
    }
    for (const QJsonValue &value : records) {
        const QJsonObject record = value.toObject();
        const QString result = record["result"].toString();
        const int delta = record["scoreDelta"].toInt();
        const QString resultClass = result == "胜利" ? "win" : result == "失败" ? "lose" : "draw";
        const QString deltaClass = delta > 0 ? "positive" : delta < 0 ? "negative" : "";
        const QString suffix = record["forcedEnd"].toBool() ? "（中途结束）" : "";
        html += QString("<tr><td>%1</td><td>%2</td><td>%3 : %4</td>"
                        "<td class=\"%5\">%6%7</td><td class=\"%8\">%9</td></tr>")
                    .arg(record["createdAt"].toString().toHtmlEscaped())
                    .arg(record["opponentName"].toString().toHtmlEscaped())
                    .arg(record["myScore"].toInt())
                    .arg(record["opponentScore"].toInt())
                    .arg(resultClass)
                    .arg(result.toHtmlEscaped())
                    .arg(suffix)
                    .arg(deltaClass)
                    .arg(delta > 0 ? QString("+%1").arg(delta) : QString::number(delta));
    }
    html += "</tbody></table>";

    auto *dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle("历史排位战绩");
    dialog->resize(760, 480);
    auto *layout = new QVBoxLayout(dialog);
    auto *browser = new QTextBrowser(dialog);
    browser->setHtml(html);
    layout->addWidget(browser);
    auto *closeButton = new QPushButton("关闭", dialog);
    closeButton->setFixedWidth(100);
    layout->addWidget(closeButton, 0, Qt::AlignRight);
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);
    dialog->open();
}

//排位赛定时器
void BrainStorm::rankTimerOut()
{
    _rankSec--;
    if(_rankSec == 0)
    {
        if(ui->rankSelectButton1->isEnabled())
            _currentRankQuestion++;
        rankSetQuestion();
        _rankSec = 10;

    }
    ui->lcdNumber_2->display(_rankSec);

}

//设置对手得分
void BrainStorm::setEnemyScore()
{
    QString str = QString("%1(%2): %3").arg(_enemyName, -5).arg(_enemyRank).arg(_enemyScore);
    ui->enemyStatus->setText(str);

}

void BrainStorm::setSelfScore()
{
    QString str = QString("%1(%2): %3").arg(_userName, -5).arg(_userRank).arg(_myScore);
    ui->selfStatus->setText(str);
}

//rank设置问题
void BrainStorm::rankSetQuestion()
{
    if(_currentRankQuestion >= QUESTION_NUM)
    {
        _rankTimer.stop();
        QJsonObject json;
        json["cmd"] = RANKRESULT;
        json["score"] = _myScore;
        json["enemyName"] = _enemyName;
        json["enemyScore"] = _enemyScore;
        _com->writeData(json);
        return;
    }

    ui->rankQuestion->setText(_rankQuestion["question"].toArray().at(_currentRankQuestion).toString());

    ui->rankSelectButton1->setText(_rankQuestion["selection1"].toArray().at(_currentRankQuestion).toString());
    ui->rankSelectButton2->setText(_rankQuestion["selection2"].toArray().at(_currentRankQuestion).toString());
    ui->rankSelectButton3->setText(_rankQuestion["selection3"].toArray().at(_currentRankQuestion).toString());
    ui->rankSelectButton4->setText(_rankQuestion["selection4"].toArray().at(_currentRankQuestion).toString());

    ui->rankSelectButton1->setEnabled(true);
    ui->rankSelectButton2->setEnabled(true);
    ui->rankSelectButton3->setEnabled(true);
    ui->rankSelectButton4->setEnabled(true);

    ui->rankSelectButton1->setStyleSheet("");
    ui->rankSelectButton2->setStyleSheet("");
    ui->rankSelectButton3->setStyleSheet("");
    ui->rankSelectButton4->setStyleSheet("");

}

//rank答题
void BrainStorm::rankAnswerQuestion(int select)
{
    //计算得分
    if(select == _rankQuestion["answer"].toArray().at(_currentRankQuestion).toString().toInt())
    {
        _myScore += 20;
    }

    setSelfScore();
    _currentRankQuestion++;

    //判断是否到下一题
    if(_currentRankQuestion == _enemyRankQuestion)
    {
        _rankSec = 10;
        _rankTimer.stop();
        ui->lcdNumber_2->display(_rankSec);
        _rankTimer.start(1000);
        rankSetQuestion();
    }


    QJsonObject json;
    json["cmd"] = ANSWER;
    json["enemyName"] = _enemyName;
    json["score"] = _myScore;
    json["questionId"] = _currentRankQuestion;

    _com->writeData(json);

}

//rank结果
void BrainStorm::rankSetResult(QJsonObject &json)
{
    _rankTimer.stop();
    QString newRank = json["newRank"].toString();
    if(json["forcedWin"].toBool())
    {
        ui->rankResult->setText(json["message"].toString("对手已退出，本局获胜"));
    }
    else if(_myScore == _enemyScore)
    {
        ui->rankResult->setText("平局");
    }
    else if(_myScore < _enemyScore)
    {
        ui->rankResult->setText("失败");
    }
    else if(_myScore > _enemyScore)
    {
        ui->rankResult->setText("胜利");
    }

    QString str = QString("%1 ---> %2").arg(_userRank).arg(newRank);
    ui->newRank->setText(str);
    _userRank = newRank;

    ui->stackedWidget->setCurrentWidget(ui->rank_result);
}

void BrainStorm::Rank(QJsonObject json)
{
    int cmd = json["cmd"].toInt();
    if(cmd == RANK)
    {
        _rankQuestion = json["question"].toObject();
        _enemyName = json["enemyName"].toString();
        _enemyRank = json["enemyRank"].toString();
        _enemyScore = json["enemyScore"].toInt();

        _rankSec = 10;
        _myScore = 0;
        ui->lcdNumber_2->display(_rankSec);

        _currentRankQuestion = 0;
        _enemyRankQuestion = 0;

        setEnemyScore();
        setSelfScore();
        rankSetQuestion();

        _rankTimer.start(1000);
        ui->stackedWidget->setCurrentWidget(ui->rank_running);
    }

    else if(cmd == ANSWER)
    {
        _enemyScore = json["enemyscore"].toInt();
        _enemyRankQuestion = json["enemyQuestionId"].toInt();
        setEnemyScore();

        if(_currentRankQuestion == _enemyRankQuestion)
        {
            _rankSec = 10;
            _rankTimer.stop();
            ui->lcdNumber_2->display(_rankSec);
            _rankTimer.start(1000);
            rankSetQuestion();
        }
    }

    else if(cmd == RANKRESULT)
    {
        rankSetResult(json);
    }
}


//排位赛的四个选项
void BrainStorm::on_rankSelectButton1_clicked()
{
    ui->rankSelectButton1->setStyleSheet("background-color: rgb(170, 170, 255)");
    ui->rankSelectButton1->setEnabled(false);
    ui->rankSelectButton2->setEnabled(false);
    ui->rankSelectButton3->setEnabled(false);
    ui->rankSelectButton4->setEnabled(false);

    rankAnswerQuestion(1);
}


void BrainStorm::on_rankSelectButton2_clicked()
{
    ui->rankSelectButton2->setStyleSheet("background-color: rgb(170, 170, 255)");
    ui->rankSelectButton1->setEnabled(false);
    ui->rankSelectButton2->setEnabled(false);
    ui->rankSelectButton3->setEnabled(false);
    ui->rankSelectButton4->setEnabled(false);

    rankAnswerQuestion(2);
}


void BrainStorm::on_rankSelectButton3_clicked()
{
    ui->rankSelectButton3->setStyleSheet("background-color: rgb(170, 170, 255)");
    ui->rankSelectButton1->setEnabled(false);
    ui->rankSelectButton2->setEnabled(false);
    ui->rankSelectButton3->setEnabled(false);
    ui->rankSelectButton4->setEnabled(false);

    rankAnswerQuestion(3);
}


void BrainStorm::on_rankSelectButton4_clicked()
{
    ui->rankSelectButton4->setStyleSheet("background-color: rgb(170, 170, 255)");
    ui->rankSelectButton1->setEnabled(false);
    ui->rankSelectButton2->setEnabled(false);
    ui->rankSelectButton3->setEnabled(false);
    ui->rankSelectButton4->setEnabled(false);

    rankAnswerQuestion(4);
}


void BrainStorm::on_pushButton_2_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->mainMenu);
}

void BrainStorm::on_pushButton_3_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->mainMenu);
}

