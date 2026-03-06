#include "widget.h"

// widget.cpp 实现（详细中文注释）
// 目的：管理宠物角色的逐帧动画与动作序列切换，提供“待机交替”和“大循环”两种运行模式。
// 核心数据结构与计时器：
// - `action_map`：映射 `RoleAct` -> 对应帧图片路径列表（QList<QString>），每个动作通过多张图片组成动画。
// - `timer`：帧计时器，按 `frameIntervalMs` 周期触发，逐帧更新当前动作的显示（由 `curFrameIndex` 控制帧索引）。
// - `stateTimer`：状态计时器，用于在当前动作播放完毕后或达到持续时长时触发切换到下一个动作。
// 运行模式：
// - 默认模式：在 `StayingA` 与 `StayingB` 之间交替展示（适合持续待机效果）。
// - 大循环（BigLoop）：按 `bigLoopSequence` 列表指定的顺序依次播放动作，循环往复。
// 关键实现要点与扩展建议：
// - 动作持续时长计算为：动作帧数 * `frameIntervalMs`；若需改变动作时长，可调整资源帧数或 `frameIntervalMs`。
// - 若要修改大循环顺序，请在构造函数中调整 `bigLoopSequence` 列表。
// - 资源数量较多时注意内存与 I/O：`action_map` 仅存路径而不直接加载像素数据，实际加载在绘制时通过 QPixmap::load 完成。

Widget::Widget(QWidget *parent)
    : QWidget(parent),
      timer(new QTimer(this)),
      stateTimer(new QTimer(this)),
      menu(new QMenu(this)),
      bigLoopEnabled(false),
      frameIntervalMs(120),
      curFrameIndex(0)
{
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->installEventFilter(new dragFilter);

    // 帧计时器：按 `frameIntervalMs` 周期触发，更新当前动作的下一帧图片并重绘
    connect(timer, &QTimer::timeout, [this]() {
        auto paths = this->action_map.value(this->curRoleAction);
        if (paths.isEmpty()) return; // 没有资源则跳过
        // 选择当前帧并更新索引
        this->curRolePix = paths[this->curFrameIndex % paths.size()];
        this->curFrameIndex = (this->curFrameIndex + 1) % qMax(1, paths.size());
        this->update();
    });

    initMenu();

    loadRoleActRes();

    // default big loop sequence as required
    bigLoopSequence = QList<RoleAct>{
        RoleAct::StayingA,
        RoleAct::Stay_WriteA,
        RoleAct::WriteA,
        RoleAct::Write_SleepA,
        RoleAct::SleepA,
        RoleAct::SleepB,
        RoleAct::Write_SleepB,
        RoleAct::WriteB,
        RoleAct::Stay_WriteB,
        RoleAct::StayingB
    };

    // 先显示初始待机动作（StayingA），并启动状态切换调度。
    // 说明：这里立即启动帧计时器以保证程序启动后马上有动画在播放；
    // stateTimer 的触发由 scheduleNextState 计算并安排，负责动作间切换。
    this->curFrameIndex = 0;
    timer->start(frameIntervalMs);
    connect(stateTimer, &QTimer::timeout, [this]() {
        if (this->bigLoopEnabled) {
            // 在大循环序列中找到当前动作索引，切换到下一个
            int idx = this->bigLoopSequence.indexOf(this->curRoleAction);
            if (idx < 0) idx = 0;
            int next = (idx + 1) % this->bigLoopSequence.size();
            showActAnimation(this->bigLoopSequence[next]);
        } else {
            // 非大循环：保持在 StayingA/StayingB 之间交替
            if (this->curRoleAction == RoleAct::StayingA) showActAnimation(RoleAct::StayingB);
            else showActAnimation(RoleAct::StayingA);
        }
        // 为刚切换到的动作安排下一次状态切换时间
        this->scheduleNextState();
    });
    QIcon icon("://image/1.png");
    this->setWindowIcon(icon);
    // start the state scheduling loop
    scheduleNextState();
}

Widget::~Widget() {}

void Widget::showActAnimation(RoleAct k)
{
    timer->stop();
    this->curRoleAction = k;
    // 切换到指定动作并从其第一帧开始播放。
    // 操作步骤：
    // 1. 停止当前帧计时器，防止在重置索引时发生竞态；
    // 2. 更新当前动作枚举 `curRoleAction`；
    // 3. 重置 `curFrameIndex` 为 0，从第一帧开始播放；
    // 4. 启动 `timer` 以按帧间隔驱动逐帧播放。
    this->curFrameIndex = 0;
    timer->start(frameIntervalMs);
}

void Widget::paintEvent(QPaintEvent * event)
{
    // 绘制当前帧。
    // 说明：这里使用 `curRolePix` 路径按需加载图片，避免在内存中一次性保留大量位图。
    // 如果你希望优化性能，可考虑预加载热点动作的 QPixmap 缓存。
    QPainter painter(this);
    QPixmap pix(this->curRolePix);
    pix.load(this->curRolePix);
    painter.drawPixmap(0,0,pix);
}


void Widget::onMenuTriggered(QAction* action)
{
    QMetaEnum me = QMetaEnum::fromType<RoleAct>();
    int k = me.keyToValue(action->text().toStdString().c_str());
    if (k < 0) return;
    showActAnimation(static_cast<RoleAct>(k));
}

void Widget::contextMenuEvent(QContextMenuEvent * event)
{
    // widget.cpp 实现（中文注释）
    this->menu->popup(QCursor::pos());
}

void Widget::initMenu()
{
    // 初始化右键菜单：每项用于手动切换到对应动作，并在切换时停止大循环（以便人工观察）。
    // 菜单中包含一个可勾选的 `BigLoop` 项，用于开启/关闭按序列循环的行为。
    menu = new QMenu(this);
    menu->addAction("StayingA", this, [this]() {
        stopBigLoop();
        showActAnimation(RoleAct::StayingA);
        scheduleNextState();
    });
    menu->addAction("StayingB", this, [this]() {
        stopBigLoop();
        showActAnimation(RoleAct::StayingB);
        scheduleNextState();
    });
    menu->addAction("Stay_WriteA", this, [this]() {
        stopBigLoop();
        showActAnimation(RoleAct::Stay_WriteA);
        scheduleNextState();
    });
    menu->addAction("Stay_WriteB", this, [this]() {
        stopBigLoop();
        showActAnimation(RoleAct::Stay_WriteB);
        scheduleNextState();
    });
     menu->addAction("WriteA", this, [this]() {
        stopBigLoop();
        showActAnimation(RoleAct::WriteA);
        scheduleNextState();
    });
    menu->addAction("WriteB", this, [this]() {
        stopBigLoop();
        showActAnimation(RoleAct::WriteB);
        scheduleNextState();
    });
    menu->addAction("Write_SleepA", this, [this]() {
        stopBigLoop();
        showActAnimation(RoleAct::Write_SleepA);
        scheduleNextState();
    });
    menu->addAction("Write_SleepB", this, [this]() {
        stopBigLoop();
        showActAnimation(RoleAct::Write_SleepB);
        scheduleNextState();
    });
    menu->addAction("SleepA", this, [this]() {
        stopBigLoop();
        showActAnimation(RoleAct::SleepA);
        scheduleNextState();
    });
    menu->addAction("SleepB", this, [this]() {
        stopBigLoop();
        showActAnimation(RoleAct::SleepB);
        scheduleNextState();
    });
    // Big loop toggle (checkable)
    QAction* bigLoopAct = new QAction("BigLoop", this);
    bigLoopAct->setCheckable(true);
    connect(bigLoopAct, &QAction::toggled, this, [this](bool checked){
        if(checked) startBigLoop(); else stopBigLoop();
    });
    menu->addAction(bigLoopAct);

    QAction* act = new QAction("Hide", this);
    connect(act, &QAction::triggered, this, [this](){ this->hide(); });
    menu->addAction(act);

    connect(this->menu, &QMenu::triggered, this, [this](QAction* act){
        qDebug() << "menu act:" << act->text();
    });
}

void Widget::loadRoleActRes()
{
    this->curFrameIndex = 0;
    // 载入资源函数说明：
    // - addResA：按从小到大的索引构造路径列表（正序帧序列），适用于正向播放素材；
    // - addResB：按从大到小的索引构造路径列表（反序帧序列），适用于需要镜像或反向动画的素材。
    auto addResA = [this](RoleAct k, QString path, int countLeft, int countRight)
    {
        // 载入每个动作对应的帧图片资源
        // addResA: 按从小到大的序号依次加入路径（例如 1..7）
        QList<QString> paths;
        char buf[2560];
        for(int i = countLeft; i <= countRight; i++){
            memset(buf,0,sizeof(buf));
            sprintf_s(buf, path.toStdString().c_str(), i);
            paths.append(QString::fromUtf8(buf));
        }
        action_map.insert(k, paths);
    };

    auto addResB = [this](RoleAct k, QString path, int countLeft, int countRight)
    {
        // addResB: 按从大到小的序号加入路径（用于反向顺序素材）
        QList<QString> paths;
        char buf[2560];
        for(int i = countRight; i >= countLeft; i--){
            memset(buf,0,sizeof(buf));
            sprintf_s(buf, path.toStdString().c_str(), i);
            paths.append(QString::fromUtf8(buf));
        }
        action_map.insert(k, paths);
    };

    addResA(RoleAct::StayingA,":/image/%d.png",1,7);
    // 资源命名规则说明：默认使用 ":/image/%d.png"，其中 %d 是帧编号。
    // 若要更换资源目录或命名格式，请修改此处的 path 参数。
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

// 注意：
// - 每个动作的持续时长由该动作对应的帧数量 * `frameIntervalMs` 决定。
//   如果某些动作希望更短/更长，可以调整对应资源的帧数，或在运行时改变 `frameIntervalMs`。
// - `bigLoopSequence` 初始序列在构造函数中定义。若需修改大循环顺序或插入新的动作，
//   请在构造函数里调整 `bigLoopSequence` 的列表顺序。
void Widget::startBigLoop()
{
    // 启动大循环模式：会按 `bigLoopSequence` 中的顺序循环动作。
    // 如果当前动作不在序列中，先跳转到序列首项再开始循环。
    bigLoopEnabled = true;
    if(!bigLoopSequence.contains(curRoleAction)){
        showActAnimation(bigLoopSequence.first());
    }
    // 为当前（或已切换的）动作计算并启动下一次状态切换计时
    scheduleNextState();
}

void Widget::stopBigLoop()
{
    // 关闭大循环：回到默认的 StayingA/StayingB 交替循环模式。
    // 为避免突兀跳转，若当前不是 StayingA 或 StayingB，则先切回到 StayingA。
    bigLoopEnabled = false;
    if(curRoleAction != RoleAct::StayingA && curRoleAction != RoleAct::StayingB){
        showActAnimation(RoleAct::StayingA);
    }
    scheduleNextState();
}

void Widget::scheduleNextState()
{
    // 根据当前动作的帧数计算持续时间并启动 stateTimer（单次触发）：
    // 持续时长 = 帧数 * 每帧间隔（frameIntervalMs）。
    // 说明：当 stateTimer 触发时，会根据 bigLoopEnabled 决定切换到序列的下一项或在 StayingA/B 之间交替。
    auto paths = this->action_map.value(this->curRoleAction);
    int cnt = qMax(1, paths.size());
    int duration = cnt * frameIntervalMs;
    if(duration <= 0) duration = 1000; // 防护：保证至少 1 秒
    stateTimer->start(duration);
}
