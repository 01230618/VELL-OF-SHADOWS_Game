#include "StructDef.h"

void updateMonsterAnim(Monster &monster,int CurLevel){
    if (!monster.label)  return;

    //无论进入哪个场景，都设置动画帧

    // 站立状态
    if (monster.monsterDir == 0)
    {
        monster.label->setPixmap(monster.picStand);
        monster.monsterPicID = 0;
        return;
    }
    // 走路状态
    int PicCount = 0;
    if (monster.monsterDir == 1) { // 向下
        PicCount = monster.monsterpicDown.size();
    } else if (monster.monsterDir == 2) { // 向上
        PicCount = monster.monsterpicUp.size();
    } else if (monster.monsterDir == 3) { // 向左
        PicCount = monster.monsterpicLeft.size();
    } else if (monster.monsterDir == 4) { // 向右
        PicCount = monster.monsterpicRight.size();
    }
    monster.monsterPicID = (monster.monsterPicID + 1) % PicCount;

    QPixmap currentPic;
    // 1=下，2=上，3=左，4=右
    switch (monster.monsterDir)
    {
    case 1: // 向下
        currentPic = monster.monsterpicDown[monster.monsterPicID];
        break;
    case 2: // 向上
        currentPic = monster.monsterpicUp[monster.monsterPicID];
        break;
    case 3: // 向左
        currentPic = monster.monsterpicLeft[monster.monsterPicID];
        break;
    case 4: // 向右
        currentPic = monster.monsterpicRight[monster.monsterPicID];
        break;
    default:
        currentPic = monster.picStand;
        break;
    }
    if(monster.isHurt && CurLevel !=3)
    {
        if(monster.monsterDir==3){
            if(monster.monsterPicHurtLeft.isEmpty()){
                return;
            }
            monster.monsterHurtID = (monster.monsterHurtID + 1) % monster.monsterPicHurtLeft.size();
            currentPic = monster.monsterPicHurtLeft[monster.monsterHurtID];
        }
        else if(monster.monsterDir==4){
            if(monster.monsterPicHurtRight.isEmpty()){
                return;
            }
            monster.monsterHurtID = (monster.monsterHurtID + 1) % monster.monsterPicHurtRight.size();
            currentPic = monster.monsterPicHurtRight[monster.monsterHurtID];
        }
        else{
            if(monster.monsterPicHurtFront.isEmpty()){
                return;
            }
            monster.monsterHurtID = (monster.monsterHurtID + 1) % monster.monsterPicHurtFront.size();
            currentPic = monster.monsterPicHurtFront[monster.monsterHurtID];
        }
    }
    if(CurLevel ==3 &&monster.isAlive && monster.isFighting){
        if(monster.monsterDir==3){
            if(monster.monsterPicFightLeft.isEmpty()){
                return;
            }
            monster.monsterFightID = (monster.monsterFightID + 1) % monster.monsterPicFightLeft.size();
            currentPic = monster.monsterPicFightLeft[monster.monsterFightID];
        }
        else if(monster.monsterDir==4){
            if(monster.monsterPicFightRight.isEmpty()){
                return;
            }
            monster.monsterFightID = (monster.monsterFightID + 1) % monster.monsterPicFightRight.size();
            currentPic = monster.monsterPicFightRight[monster.monsterFightID];
        }
    }
    monster.label->setPixmap(currentPic);
}

//初始化怪物血量
void iniMonsterHP(Monster &m,QWidget *parent)
{
    //血条底板
    m.monsterHpBg = new QLabel(parent);
    m.monsterHpBg->setFixedSize(70, 8);
    m.monsterHpBg->setStyleSheet(R"(
    background-color: #222222;
    border-radius: 4px;
)");

    //血量条
    m.monsterHpbar = new QLabel(parent);
    m.monsterHpbar->setFixedSize(66, 6);
    m.monsterHpbar->setStyleSheet(R"(
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
    stop:0 #a82020, stop:1 #d13030);
    border-radius: 3px;
)");
    //初始位置
    m.monsterHpBg->move(m.monster_x, m.monster_y - 18);
    m.monsterHpbar->move(m.monster_x + 2, m.monster_y - 17);
    m.monsterHpBg->lower();
    m.monsterHpbar->raise();
    m.monsterHpBg->show();
    m.monsterHpbar->show();
}
