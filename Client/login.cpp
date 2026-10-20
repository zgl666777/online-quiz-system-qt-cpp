#include "login.h"
#include "ui_login.h"
#include "register.h"
#include "brainstorm.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);


    _com = new Communicate();

    connect(_com, SIGNAL(login(QJsonObject )), this, SLOT(login(QJsonObject )));
    connect(_com, &Communicate::connectionError, this, [this](const QString &message) {
        QMessageBox::warning(this, "网络连接", message);
    });

}

Widget::~Widget()
{
    delete ui;
}

 //登录按键
void Widget::on_pushButton_clicked()
{
    const QString userName = ui->lineEdit->text().trimmed();
    const QString passwd = ui->lineEdit_2->text();
    if (userName.isEmpty() || passwd.isEmpty()) {
        QMessageBox::warning(this, "登录", "请输入用户名和密码。");
        return;
    }
    if (!_com->isConnected()) {
        QMessageBox::warning(this, "登录", "尚未连接到答题服务器，请稍后重试。");
        return;
    }

    //将登录信息发送给服务器
    QJsonObject json;
    json["cmd"] = LOGIN;
    json["userName"] = userName;
    json["passwd"] = passwd;

    //给服务器发送数据
    _com->writeData(json);


}

void Widget::login(QJsonObject json)
{
    int result = json["result"].toInt();

    switch (result)
    {
    case OK:
    {
        this->hide();
        BrainStorm *dlg = new BrainStorm(_com, json);
        dlg->show();
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        break;
    }
    case ERROR:
        QMessageBox::information(this, "登录", "登录失败，未知错误！");
        break;
    case USERLOGIN:
        QMessageBox::information(this, "登录", "登录失败，不允许重复登录！");
        break;
    case NAMEORPASSWD:
        QMessageBox::information(this, "登录", "用户名或密码错误");
    default:
        break;
    }


}

//注册按键
void Widget::on_pushButton_2_clicked()
{
    this->hide();
    Register reg(_com);
    reg.exec();
    this->show();


}

