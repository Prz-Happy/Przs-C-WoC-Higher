#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent),
    timer(new QTimer(this)),
    menu(new QMenu(this))
{

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->installEventFilter(new dragFilter);

    connect(timer,&QTimer::timeout,[this]()
    {
        static int index=0;
        auto paths = this->action_map.value(this->curRoleAction);
        this->curRolePix=paths[index++%paths.size()];
        this->update();
    });

    initMenu();

    loadRoleActRes();

    showActAnimation(RoleAct::StayingA);
    //showActAnimation(RoleAct::StayingB);
}

Widget::~Widget() {}

void Widget::showActAnimation(RoleAct k)
{
    timer->stop();
    this->curRoleAction=k;
    timer->start(120);
}

void Widget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    QPixmap pix(this->curRolePix);
    pix.load(this->curRolePix);
    painter.drawPixmap(0,0,pix);
}


void Widget::onMenuTriggered(QAction* action)
{
    QMetaEnum me=QMetaEnum::fromType<RoleAct>();
    bool ok;
    int k=me.keyToValue(action->text().toStdString().c_str());
    if(!ok)return;
    showActAnimation(static_cast<RoleAct>(k));
}

void Widget::contextMenuEvent(QContextMenuEvent * event)
{
    this->menu->popup(QCursor::pos());
}

void Widget::initMenu()
{
    menu=new QMenu(this);
    menu->addAction("StayingA",this,[this]()
    {
        showActAnimation(RoleAct::StayingA);
    });
    menu->addAction("StayingB",this,[this]()
    {
        showActAnimation(RoleAct::StayingB);
    });

    QAction* act=new QAction("Hide",this);
    connect(act,&QAction::triggered,this,[this](){
        this->hide();
    });

    menu->addAction(act);

    connect(this->menu,&QMenu::triggered,this,[this](QAction* act){
        qDebug()<<"menu act:"<<act->text();
    });
}


void Widget::loadRoleActRes()
{

    auto addResA=[this](RoleAct k,QString path,int countLeft,int countRight)
    {
        QList<QString> paths;
        char buf[2560];
        for(int i=countLeft; i<=countRight;i++ ){
            memset(buf,0,sizeof(buf));
            sprintf_s(buf,path.toStdString().c_str(),i);
            paths.append(QString::fromUtf8(buf));
        }
        action_map.insert(k,paths);
    };

    auto addResB=[this](RoleAct k,QString path,int countLeft,int countRight)
    {
        QList<QString> paths;
        char buf[2560];
        for(int i=countRight; i>=countLeft;i-- ){
            memset(buf,0,sizeof(buf));
            sprintf_s(buf,path.toStdString().c_str(),i);
            paths.append(QString::fromUtf8(buf));
        }
        action_map.insert(k,paths);
    };

    addResA(RoleAct::StayingA,":/image/%d.png",1,7);
    addResB(RoleAct::StayingB,":/image/%d.png",1,7);
    addResA(RoleAct::Stay_WriteA,":/image/%d.png",8,126);
    addResB(RoleAct::Stay_WriteB,":/image/%d.png",8,126);
    addResA(RoleAct::WriteA,":/image/%d.png",127,210);
    addResB(RoleAct::WriteB,":/image/%d.png",127,210);
    addResA(RoleAct::Write_SleepA,":/image/%d.png",211,223);
    addResB(RoleAct::Write_SleepB,":/image/%d.png",211,223);
    addResA(RoleAct::SleepA,":/image/%d.png",224,239);
    addResB(RoleAct::SleepB,":/image/%d.png",224,239);

}

