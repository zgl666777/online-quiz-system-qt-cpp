#include "register.h"
#include "ui_register.h"

Register::Register(Communicate *com, QWidget *parent)
    : QDialog(parent), ui(new Ui::Register), _com(com)
{
    ui->setupUi(this);
    ui->lineEdit->clear();
    ui->lineEdit_2->clear();
    connect(_com, &Communicate::rsResult, this, &Register::rsResult);
}

Register::~Register()
{
    disconnect(_com, nullptr, this, nullptr);
    delete ui;
}

void Register::on_pushButton_clicked()
{
    const QString userName = ui->lineEdit->text().trimmed();
    const QString password = ui->lineEdit_2->text();
    if (userName.isEmpty() || userName.size() > 32) {
        QMessageBox::warning(this, "注册", "用户名不能为空且不能超过 32 个字符。");
        return;
    }
    if (password.size() < 4) {
        QMessageBox::warning(this, "注册", "密码至少需要 4 个字符。");
        return;
    }
    if (!_com->isConnected()) {
        QMessageBox::warning(this, "注册", "尚未连接到答题服务器，请稍后重试。");
        return;
    }

    QJsonObject json;
    json["cmd"] = REGISTER;
    json["userName"] = userName;
    json["passwd"] = password;
    _com->writeData(json);
}

void Register::rsResult(int result)
{
    switch (result) {
    case OK:
        QMessageBox::information(this, "注册", "注册成功，请返回登录。");
        accept();
        break;
    case USEREXIST:
        QMessageBox::information(this, "注册", "该用户名已经存在，请更换用户名。");
        break;
    default:
        QMessageBox::information(this, "注册", "注册失败，请检查用户名和密码。");
        break;
    }
}

void Register::on_pushButton_2_clicked()
{
    reject();
}
