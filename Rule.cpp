#include "Rule.h"
#include "ui_Rule.h"

Rule::Rule(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Rule)
{
    ui->setupUi(this);

    QString textcontents=("1. 开局可自由选择角色皮肤，开启专属冒险；\n"
                            "\n"
                            "2. 移动操作：使用 WSAD 或方向键控制人物行走；\n"
                            "\n"
                            "3. 战斗按键：J 向左攻击、L 向右攻击、K 防御、I 隐身；\n"
                            "\n"
                            "4. 关卡机制：初始仅解锁第一关，通关当前关卡即可解锁下一关；\n"
                            "\n"
                            "5. 通关目标：依次攻克全部三关，即可达成游戏胜利；\n"
                            "\n"
                            "6. 场景特色：每一处关卡都拥有专属机制与独特危机，玩法各不相同。\n"
                            "\n"
                            "                   准备就绪，即刻踏入秘境，开启冒险之旅！");
    ui->text_label->setText(textcontents);
}

Rule::~Rule()
{
    delete ui;
}

void Rule::on_btn_closed_clicked()
{
    this->close();
}

