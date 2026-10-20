#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include "communicate.h"
#include "../common.h"
#include <QMessageBox>


QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void login(QJsonObject json);

private:
    Ui::Widget *ui;

    Communicate *_com;          //通信类指针 和服务器通信

};
#endif // LOGIN_H
