#include "communicate.h"
#include <QDataStream>
#include <QHostAddress>
#include <QJsonDocument>

Communicate::Communicate(QObject *parent) : QObject(parent)
{
    const QString host = qEnvironmentVariable("QUIZ_SERVER_HOST", "192.168.63.128");
    const quint16 port = qEnvironmentVariableIntValue("QUIZ_SERVER_PORT") > 0
                             ? qEnvironmentVariableIntValue("QUIZ_SERVER_PORT") : 9999;
    connect(&s, &QTcpSocket::readyRead, this, &Communicate::readData);
    connect(&s, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        emit connectionError(s.errorString());
    });
    s.connectToHost(host, port);
}

void Communicate::readData()
{
    buffer += s.readAll();
    while (true) {
        if (expectedSize == 0) {
            if (buffer.size() < 4) return;
            QDataStream stream(buffer.left(4));
            stream.setByteOrder(QDataStream::BigEndian);
            stream >> expectedSize;
            buffer.remove(0, 4);
            if (expectedSize == 0 || expectedSize > 1024 * 1024) {
                expectedSize = 0;
                buffer.clear();
                return;
            }
        }
        if (buffer.size() < static_cast<int>(expectedSize)) return;
        const QByteArray payload = buffer.left(expectedSize);
        buffer.remove(0, expectedSize);
        expectedSize = 0;

        QJsonParseError error;
        const QJsonDocument document = QJsonDocument::fromJson(payload, &error);
        if (error.error != QJsonParseError::NoError || !document.isObject()) continue;
        const QJsonObject root = document.object();
        switch (root["cmd"].toInt()) {
        case REGISTER: emit rsResult(root["result"].toInt()); break;
        case LOGIN: emit login(root); break;
        case SINGLE_GETQUESTION: emit receiveSingleQuestion(root["question"].toObject()); break;
        case RANK:
        case ANSWER:
        case RANKRESULT: emit Rank(root); break;
        case LEADERBOARD: emit leaderBoard(root); break;
        case MATCH_HISTORY: emit matchHistory(root); break;
        default: break;
        }
    }
}

void Communicate::writeData(const QJsonObject &json)
{
    if (s.state() != QAbstractSocket::ConnectedState) {
        emit connectionError(QStringLiteral("无法连接答题服务器，请确认 Ubuntu 服务已启动。"));
        return;
    }
    const QByteArray payload = QJsonDocument(json).toJson(QJsonDocument::Compact);
    QByteArray frame;
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint32>(payload.size());
    frame += payload;
    s.write(frame);
}

bool Communicate::isConnected() const
{
    return s.state() == QAbstractSocket::ConnectedState;
}
