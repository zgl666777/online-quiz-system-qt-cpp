#ifndef COMMUNICATE_H
#define COMMUNICATE_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include "../common.h"

class Communicate : public QObject
{
    Q_OBJECT
public:
    explicit Communicate(QObject *parent = nullptr);
    void writeData(const QJsonObject &json);
    bool isConnected() const;

signals:
    void rsResult(int result);
    void login(QJsonObject json);
    void receiveSingleQuestion(QJsonObject json);
    void Rank(QJsonObject json);
    void leaderBoard(QJsonObject json);
    void matchHistory(QJsonObject json);
    void connectionError(QString message);

private slots:
    void readData();

private:
    QTcpSocket s;
    QByteArray buffer;
    quint32 expectedSize = 0;
};

#endif
