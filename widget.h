#ifndef WIDGET_H
#define WIDGET_H

#include <QMap>
#include <QMenu>
#include <QList>
#include <QString>
#include <QWidget>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QFile>
#include <QTextStream>
#include <QIcon>
#include <QmetaEnum>
#include <QSystemTrayIcon>
class QPaintEvent;

namespace Act{
    Q_NAMESPACE
    enum  RoleAct{
        StayingA,
        StayingB,
        Stay_WriteA,
        Stay_WriteB,
        WriteA,
        WriteB,
        Write_SleepA,
        Write_SleepB,
        SleepA,
        SleepB,
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

public slots:
    void onMenuTriggered(QAction* action);

protected:
    void paintEvent(QPaintEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);

private:
    QMap<RoleAct,QList<QString>> action_map;
    QTimer* timer;
    RoleAct curRoleAction;
    QString curRolePix;
    QMenu* menu;

    void loadRoleActRes();
    void showActAnimation(RoleAct k);
    void initMenu();
};

class dragFilter : public QObject
{
public:
    bool eventFilter(QObject* obj, QEvent* event) 
    {
        auto where = dynamic_cast<QWidget*>(obj);

        if (!where) return false;

        if(event->type() == QEvent::MouseButtonPress)
        {
            auto  eventMouse = dynamic_cast<QMouseEvent*>(event);
            if(eventMouse){
                pos=eventMouse->pos();
            }
            return true;
        }

        else if(event->type() == QEvent::MouseMove)
        {
            auto  eventMouse = dynamic_cast<QMouseEvent*>(event);
            if(eventMouse) {
                if(eventMouse->buttons() & Qt::MouseButtons(Qt::LeftButton)) {
                    where->move(eventMouse->globalPosition().toPoint() - pos);
                }
            }
        }

        return QObject::eventFilter(obj, event);
    }
private:
    QPoint pos;
};
#endif // WIDGET_H
