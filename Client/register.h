#ifndef REGISTER_H
#define REGISTER_H

#include <QDialog>
#include <QJsonObject>
#include "communicate.h"
#include "../common.h"
#include <QMessageBox>

namespace Ui {
class Register;
}

class Register : public QDialog
{
    Q_OBJECT

public:
    explicit Register(Communicate *com,QWidget *parent = nullptr);
    ~Register();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void rsResult(int );

private:
    Ui::Register *ui;

    Communicate *_com;
};

#endif // REGISTER_H
