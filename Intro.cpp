#include "Intro.h"
#include "ui_Intro.h"

Intro::Intro(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Intro)
{
    ui->setupUi(this);

    //介绍页面大小，图层
    this->setFixedSize(960,540);
    this->setWindowModality(Qt::ApplicationModal);//挡住下层游戏

    //介绍页面背景
    ui->background->setPixmap(QPixmap(":/ui/introBg.png"));
    ui->background->setScaledContents(true);
    ui->background->show();

    //按钮使用样式表—ui界面里设置的
}

Intro::~Intro()
{
    delete ui;
}

//给widget里的 intro->ui->introduction 设置文本
void Intro::setIntroText(const QString& text){
    ui->introduction->setText(text);
    ui->introduction->setAlignment(Qt::AlignCenter);//居中
}

void Intro::on_start_clicked()
{
    emit startGame();       //发送开始游戏信号
    this->close();          //关闭介绍页面
}

