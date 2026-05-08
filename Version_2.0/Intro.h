#ifndef INTRO_H
#define INTRO_H

#include <QWidget>

namespace Ui {
class Intro;
}

class Intro : public QWidget
{
    Q_OBJECT

public:
    explicit Intro(QWidget *parent = nullptr);
    ~Intro();

    //给介绍页面设置文本
    void setIntroText(const QString& text);
    //因为widget无法调用intro的ui(private),所以需要一个public函数来设置文本

signals:
    void startGame();  //游戏开始信号，从intro页面的关闭告诉widget游戏可以开始

private slots:
    void on_start_clicked();//“开始闯关”

private:
    Ui::Intro *ui;
};

#endif // INTRO_H
