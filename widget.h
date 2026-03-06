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

// Widget 类说明（中文注释）:
// - 本窗口显示角色动作动画，通过两类计时器协调：
//   1) `timer`：按帧间隔逐帧切换当前动作的图片，实现动画播放；
//   2) `stateTimer`：控制动作间的切换时长，动作播放完毕或时间到后由其触发下一动作切换。
// - 支持两种运行模式：普通待机交替（StayingA <-> StayingB）和“大循环”模式（按预设动作序列循环）。
// - 右键菜单可手动切换动作，或勾选 `BigLoop` 启用/关闭大循环。

namespace Act{
    Q_NAMESPACE
    enum  RoleAct{
        // 待机/站立状态 A（默认短循环的一端）
        StayingA,
        // 待机/站立状态 B（默认短循环的另一端）
        StayingB,
        // 从待机到写字（A 侧开始切换前的准备帧序列）
        Stay_WriteA,
        // 从待机到写字（B 侧准备序列）
        Stay_WriteB,
        // 写字动作 A（持续多帧）
        WriteA,
        // 写字动作 B（持续多帧）
        WriteB,
        // 写字到睡觉的过渡（A 侧）
        Write_SleepA,
        // 写字到睡觉的过渡（B 侧）
        Write_SleepB,
        // 睡觉动作 A
        SleepA,
        // 睡觉动作 B
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
    // 菜单触发处理（保留兼容接口）：当需要根据 QAction 文本切换动作时可用
    void onMenuTriggered(QAction* action);

protected:
    void paintEvent(QPaintEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);

private:
    QMap<RoleAct,QList<QString>> action_map;
    // 用于帧动画更新的计时器：每次触发更新当前动作的下一帧
    QTimer* timer;
    // 用于状态切换的计时器：控制从一个动作切换到下一个动作的时长
    QTimer* stateTimer;
    // 当前动作枚举（当前正在播放的动作类型）
    RoleAct curRoleAction;
    // 当前要绘制的图片路径（当前帧的路径）
    QString curRolePix;
    // 右键菜单对象
    QMenu* menu;
    // 是否启用“大循环”模式（按指定序列循环所有动作）
    bool bigLoopEnabled;
    // 大循环时的动作序列，按需求顺序填写
    QList<RoleAct> bigLoopSequence;
    // 每帧之间的间隔（毫秒）
    int frameIntervalMs;
    // 当前动作的帧索引用于在 action_map 中索引当前帧
    int curFrameIndex;

    void loadRoleActRes();
    // 加载所有动作对应的图片序列到 `action_map` 中
    void showActAnimation(RoleAct k);
    // 切换并播放指定动作（重置帧索引并启动帧计时器）
    void initMenu();
    // 初始化右键菜单及其动作项
    void startBigLoop();
    // 启动大循环模式（按 bigLoopSequence 顺序循环动作）
    void stopBigLoop();
    // 停止大循环，回到 StayingA/StayingB 的交替模式
    void scheduleNextState();
    // 根据当前动作帧数与每帧间隔计算持续时间，并启动 stateTimer 来在合适时机切换到下一个动作
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
