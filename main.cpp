 #include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;

    QSystemTrayIcon sysTray(QIcon("://image/1.png"));

    QMenu menu;
    auto showAct=new QAction("Show",&sysTray);
    auto exitAct=new QAction("Exit",&sysTray);

    QObject::connect(showAct,&QAction::triggered,[&](){
        w.setVisible(true);
    });
    QObject::connect(exitAct,&QAction::triggered,[&](){
        QApplication::quit();
    });

    menu.addAction(showAct);
    menu.addAction(exitAct);

    sysTray.setContextMenu(&menu);
    sysTray.show();

    w.show();
    return a.exec();
}
