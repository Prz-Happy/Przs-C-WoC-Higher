#ifndef WIDGET_H
#define WIDGET_H
#include <QMap>
#include <QList>
#include <QString>
#include <QWidget>
#include <QTimer>
class QPaintEvent;

namespace Act{
    Q_NAMESPACE
    enum  RoleAct{
        Sleep,
        Write,
        Prepare,
        Staying
    };
    Q_ENUM_NS(RoleAct)
}

using namespace Act;

class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
protected:
    void paintEvent(QPaintEvent* event)override;
private:
    QMap<RoleAct,QList<QString>> action_map;
    QTimer* timer;
    RoleAct curRoleAction;
    QString curRolePix;

    void loadRoleActRes();
    void showActAnimation(RoleAct k);
};

#endif // WIDGET_H
