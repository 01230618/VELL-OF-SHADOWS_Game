#include "Roleselect.h"
#include "ui_Roleselect.h"

RoleSelect::RoleSelect(QWidget *parent)
    : QDialog(parent)      //模态对话框，父类选择了QDialog
                           //因为想实现 这个窗口未关闭之前 玩家不能点开其他窗口 以这个窗口为优先
    , ui(new Ui::RoleSelect)
{
    ui->setupUi(this);
    //角色选择页面背景设置
    ui->label_bg->setPixmap(QPixmap(":/images/label_roleditu.png"));
    ui->label_bg->setScaledContents(1);
    ui->label_bg->lower();
    ui->label_bg->show();
    //按钮使用样式表设计

    curIndex=1;      //打开角色选择页面的当前角色为1号
    selectIndex=1;

    updateRoleimg();
}

RoleSelect::~RoleSelect()
{
    delete ui;
}

void RoleSelect::updateRoleimg(){
    QString rolePicture;
    switch (curIndex) {
    case 1:
        rolePicture = ":/images/role_luwuyi.png";
        break;
    case 2:
        rolePicture = ":/images/role_wushiguang.png";
        break;
    case 3:
        rolePicture = ":/images/role_wuwangyan.png";
        break;
    default:
        rolePicture = ":/images/role_jilin.png";
    }
    ui->label_roleimg->setPixmap(QPixmap(rolePicture));
}

void RoleSelect::on_left_btn_clicked()
{
    curIndex--;
    if(curIndex<=0){
        curIndex=maxRole;
    }
    updateRoleimg();
}

void RoleSelect::on_right_btn_clicked()
{
    curIndex++;
    if(curIndex>maxRole){
        curIndex=1;
    }
    updateRoleimg();
}


void RoleSelect::on_confirm_btn_clicked()
{
    selectIndex=curIndex;
    accept();
}


void RoleSelect::on_cancel_btn_clicked()
{
    reject();
}

int RoleSelect::getRole(){
    return selectIndex;
}