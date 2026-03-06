#include "widget.h"
#include<QPaintEvent>
#include<QPainter>
#include<QString>
#include<QPixmap>
#include<QDebug>
#include<QFile>
#include<QTextStream>
#include<QIcon>
Widget::Widget(QWidget *parent)
    : QWidget(parent),
    timer(new QTimer(this))
{
    connect(timer,&QTimer::timeout,[this](){
        static int index=0;//记录显示动作的图片序号
        auto paths = this->action_map.value(this->curRoleAction);
        this->curRolePix=paths[index++%paths.size()];
        //paintEvent() 不允许
        this->update();
    });
    loadRoleActRes();
    showActAnimation(RoleAct::Write);
    QIcon icon(":/image/1.png");//格式要求
    this->setWindowIcon(icon);
}

Widget::~Widget() {}

void Widget::showActAnimation(RoleAct k)
{
    timer->stop();
    this->curRoleAction=k;
    timer->start(120);
}

void Widget::paintEvent(QPaintEvent * event){
    QPainter painter(this);
    QPixmap pix(this->curRolePix);
    pix.load(this->curRolePix);
    painter.drawPixmap(0,0,pix);
}

void Widget::loadRoleActRes()
{
    auto addRes=[this](RoleAct k,QString path,int countLeft,int countRight){
        QList<QString> paths;
        char buf[2560];
        for(int i=countLeft; i<=countRight;i++ ){
            memset(buf,0,sizeof(buf));
            sprintf_s(buf,path.toStdString().c_str(),i);
            paths.append(QString::fromUtf8(buf));
        }
        action_map.insert(k,paths);
    };
    addRes(RoleAct::Staying,":/image/%d.png",0,7);
    addRes(RoleAct::Prepare,":/image/%d.png",8,9);
    addRes(RoleAct::Prepare,":/image/%d.png",10,25);
    addRes(RoleAct::Write,":/image/%d.png",26,99);
    addRes(RoleAct::Write,":/image/%d.png",100,202);
    addRes(RoleAct::Sleep,":/image/%d.png",203,239);
}

