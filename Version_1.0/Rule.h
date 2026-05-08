#ifndef RULE_H
#define RULE_H

#include <QWidget>

namespace Ui {
class Rule;
}

class Rule : public QWidget
{
    Q_OBJECT

public:
    explicit Rule(QWidget *parent = nullptr);
    ~Rule();

private slots:
    void on_btn_closed_clicked();//“关闭规则”

private:
    Ui::Rule *ui;
};

#endif // RULE_H
