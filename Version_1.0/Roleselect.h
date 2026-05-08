#ifndef ROLESELECT_H
#define ROLESELECT_H

//#include <QWidget>
#include <QPixmap>
#include <QDialog>

namespace Ui {
class RoleSelect;
}

class RoleSelect : public QDialog
{
    Q_OBJECT

public:
    explicit RoleSelect(QWidget *parent = nullptr);
    ~RoleSelect();
    int getRole();//获取角色编号，让widget接收到Roleselect——private里的selectIndex

private slots:

    void on_left_btn_clicked();

    void on_right_btn_clicked();

    void on_confirm_btn_clicked();

    void on_cancel_btn_clicked();

private:
    void updateRoleimg(); //更新角色图片
    int curIndex; //当前显示的角色编号
    int selectIndex; //最终选择的角色编号
    const int maxRole = 4; //一共四个角色

    Ui::RoleSelect *ui;
};

#endif // ROLESELECT_H
