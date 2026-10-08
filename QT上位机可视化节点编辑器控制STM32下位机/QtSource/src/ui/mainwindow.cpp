#include "mainwindow.h"                        // 本类头文件
#include "./ui_mainwindow.h"                   // uic 生成的界面代码

#include "devicenodemodel.h"                   // 下位机节点（红灯 / 蓝灯 / 蜂鸣器 / 温湿度）
#include "displaynodemodel.h"                  // 显示整数 / 显示浮点数节点
#include "flowconnectionpainter.h"             // 运行态虚线连线绘制
#include "flowgraphmodel.h"                    // 自定义图模型
#include "flowview.h"                          // 右侧画布视图
#include "ifnodemodel.h"                       // 判断节点
#include "loopnodemodel.h"                     // 循环节点
#include "nodetreeview.h"                      // 左侧节点面板
#include "outputnodemodel.h"                   // 输出整数 / 输出浮点数节点
#include "serialportmanager.h"                 // 串口管理器
#include "sleepnodemodel.h"                    // 休眠节点
#include "startnodemodel.h"                    // 开始节点模型

#include <QtCore/QDebug>                       // 日志输出
#include <QtCore/QMetaObject>                  // 跨线程调用（串口在工作线程）
#include <QtCore/QTimer>                       // 虚线动画定时器
#include <QtGui/QAction>                       // 菜单动作
#include <QtGui/QCursor>                       // 鼠标位置（兜底）
#include <QtGui/QKeySequence>                  // 快捷键
#include <QtNodes/BasicGraphicsScene>          // 场景基类（右键菜单信号）
#include <QtNodes/DataFlowGraphModel>          // 数据流图模型
#include <QtNodes/DataFlowGraphicsScene>       // 数据流场景
#include <QtNodes/NodeDelegateModelRegistry>   // 节点注册表
#include <QtNodes/internal/ConnectionGraphicsObject.hpp> // 连线图元（重绘连线）
#include <QtWidgets/QGraphicsItem>             // 场景图元基类
#include <QtSerialPort/QSerialPortInfo>        // 枚举可用串口
#include <QtWidgets/QComboBox>                 // 串口下拉框
#include <QtWidgets/QDialog>                   // 连接串口对话框
#include <QtWidgets/QDialogButtonBox>          // 对话框按钮
#include <QtWidgets/QHBoxLayout>               // 水平布局
#include <QtWidgets/QMenu>                     // 右键菜单 / 菜单栏
#include <QtWidgets/QMessageBox>               // 打开失败提示
#include <QtWidgets/QPushButton>               // 刷新按钮
#include <QtWidgets/QVBoxLayout>               // 垂直布局

// 构造函数：搭建界面与节点编辑器
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)                      // 构造基类
    , ui(new Ui::MainWindow)                   // 创建界面对象
{
    ui->setupUi(this);                         // 加载 .ui 布局

    // 菜单栏样式
    ui->menubar->setStyleSheet(R"(
    /* 顶部菜单栏 - 白色背景 */
    QMenuBar {
        background-color: #ffffff;
        color: #222222;
        font-size: 12px;
    }
    QMenuBar::item {
        background: transparent;
        padding: 0 8px;
        margin: 0;
    }
    QMenuBar::item:selected {
        background-color: #e6e6e6; /* 悬浮浅灰，不突兀 */
    }

    /* 弹出下拉菜单，深色，和白色菜单栏区分 */
    QMenu {
        background-color: #f0f0f0;
        color: #222222;
        border: 1px solid #cccccc;
    }
    QMenu::item {
        padding: 5px 16px;
        background-color: transparent;
    }
    QMenu::item:selected {
        background-color: #d8d8d8;
    }
    )");

    setupNodeEditor();                         // 初始化节点编辑器
    setupRunMenu();                            // 初始化「运行」菜单
    setupSerialMenu();                         // 初始化「串口」菜单
}

// 析构函数
MainWindow::~MainWindow()
{
    delete ui;                                 // 释放界面对象
}

// 初始化节点编辑器：注册模型并组装模型/场景/视图
void MainWindow::setupNodeEditor()
{
    _registry = std::make_shared<QtNodes::NodeDelegateModelRegistry>(); // 创建节点注册表
    registerNodeModels();                                               // 注册所有节点模型

    _graphModel = std::make_unique<FlowGraphModel>(_registry);              // 创建自定义图模型
    _scene = std::make_unique<QtNodes::DataFlowGraphicsScene>(*_graphModel, this); // 创建场景
    // 换用自定义连线绘制：运行态下把连线画成流动虚线
    auto connectionPainter = std::make_unique<FlowConnectionPainter>();  // 创建绘制器
    _connectionPainter = connectionPainter.get();                        // 记录裸指针（所有权归场景）
    _scene->setConnectionPainter(std::move(connectionPainter));          // 交给场景接管
    // 节点右键菜单：连接信号以提供删除操作
    connect(_scene.get(), &QtNodes::BasicGraphicsScene::nodeContextMenu,
            this, &MainWindow::onNodeContextMenu);
    _view = new FlowView(_scene.get(), this);                           // 创建画布视图

    auto *canvasLayout = new QVBoxLayout(ui->widget);                   // 右侧容器布局
    canvasLayout->setContentsMargins(0, 0, 0, 0);                       // 去掉边距
    canvasLayout->addWidget(_view);                                     // 放入画布视图

    _nodeTreeView = new NodeTreeView(ui->nodePanel);                    // 创建左侧节点面板
    auto *panelLayout = new QVBoxLayout(ui->nodePanel);                 // 左侧容器布局
    panelLayout->setContentsMargins(0, 0, 0, 0);                        // 去掉边距
    panelLayout->addWidget(_nodeTreeView);                              // 放入节点面板
    _nodeTreeView->setRegistry(_registry);                              // 面板按注册表生成分类与节点项
}

// 注册所有可用节点模型（新增节点类型在此加一行即可）
void MainWindow::registerNodeModels()
{
    _registry->registerModel<StartNodeModel>(QStringLiteral("流程节点"));   // 注册开始节点
    _registry->registerModel<IfModel>(QStringLiteral("流程节点"));          // 注册判断节点
    _registry->registerModel<LoopModel>(QStringLiteral("流程节点"));        // 注册循环节点
    _registry->registerModel<SleepModel>(QStringLiteral("流程节点"));       // 注册休眠节点
    _registry->registerModel<OutputIntModel>(QStringLiteral("输出节点"));   // 注册输出整数
    _registry->registerModel<OutputFloatModel>(QStringLiteral("输出节点")); // 注册输出浮点数
    _registry->registerModel<DisplayIntModel>(QStringLiteral("显示节点"));  // 注册显示整数
    _registry->registerModel<DisplayFloatModel>(QStringLiteral("显示节点")); // 注册显示浮点数
    _registry->registerModel<RedLedModel>(QStringLiteral("下位机节点"));      // 注册红灯
    _registry->registerModel<BlueLedModel>(QStringLiteral("下位机节点"));     // 注册蓝灯
    _registry->registerModel<BuzzerModel>(QStringLiteral("下位机节点"));      // 注册蜂鸣器
    _registry->registerModel<TempHumiModel>(QStringLiteral("下位机节点"));    // 注册温湿度
}

// 初始化「运行」菜单：运行 / 停止 两个动作
void MainWindow::setupRunMenu()
{
    ui->menubar->setNativeMenuBar(false);      // 改用窗口内菜单栏（而非系统菜单栏）
    ui->menubar->setFixedHeight(26);           // 收窄高度，避免 macOS 样式下菜单栏过高
    // 给菜单项加上底线底色与悬停高亮，避免与白色菜单栏融为一体
    ui->menubar->setStyleSheet(QStringLiteral(
        "QMenuBar {"
        "  background: #f7f8fa;"                    // 菜单栏底色（浅灰）
        "  padding: 0px;"                           // 去掉内边距
        "  border-bottom: 1px solid #d5d8dd;"       // 底部分隔线
        "}"
        "QMenuBar::item {"
        "  background: #e4e7ec;"                    // 常态底色，与白底区分开
        "  color: #2b2b2b;"                         // 常态文字色
        "  padding: 1px 12px;"                      // 内边距
        "  margin: 2px;"                            // 外边距（项之间的间隔）
        "  border-radius: 4px;"                     // 圆角
        "}"
        "QMenuBar::item:selected {"
        "  background: #4a86e8;"                    // 悬停/展开时的底色
        "  color: #ffffff;"                         // 悬停/展开时的文字色
        "}"
        "QMenuBar::item:pressed {"
        "  background: #3a6dc4;"                    // 按下时的底色
        "  color: #ffffff;"                         // 按下时的文字色
        "}"));
    QMenu *runMenu = ui->menubar->addMenu(QStringLiteral("运行"));      // 顶部“运行”菜单
    _runAction = runMenu->addAction(QStringLiteral("运行"));            // 运行动作
    _runAction->setShortcut(QKeySequence(QStringLiteral("F5")));        // F5 快捷键
    _stopAction = runMenu->addAction(QStringLiteral("停止"));           // 停止动作
    _stopAction->setShortcut(QKeySequence(QStringLiteral("Shift+F5"))); // Shift+F5 快捷键
    _stopAction->setEnabled(false);                                     // 初始为停止态

    connect(_runAction, &QAction::triggered, this, &MainWindow::onRun);   // 连接运行动作
    connect(_stopAction, &QAction::triggered, this, &MainWindow::onStop); // 连接停止动作
    // 虚线动画定时器：运行态下持续刷新连线的虚线偏移
    _animationTimer = new QTimer(this);                                     // 创建定时器
    _animationTimer->setInterval(60);                                       // 约 16 帧/秒
    connect(_animationTimer, &QTimer::timeout, this, &MainWindow::onAnimationTick); // 定时推进动画
    // 状态变化时刷新两个动作的可用性，并切换连线的动态虚线外观
    connect(_graphModel.get(), &FlowGraphModel::runStateChanged, this,
            [this](FlowGraphModel::RunState state) {
                bool const running = state == FlowGraphModel::RunState::Running; // 是否运行态
                _runAction->setEnabled(!running);                       // 运行中禁用“运行”
                _stopAction->setEnabled(running);                       // 停止中禁用“停止”
                if (!running)                                           // 从运行态转为停止态
                    QMetaObject::invokeMethod(&SerialPortManager::instance(), []() {
                        SerialPortManager::instance().resetOutputs();   // 在工作线程里下发 0x00 关闭所有外设
                    }, Qt::QueuedConnection);                            // 异步，不阻塞 UI
                if (_connectionPainter)                                 // 绘制器已就绪
                    _connectionPainter->setRunning(running);            // 切换运行态外观
                if (_animationTimer) {                                  // 定时器已就绪
                    if (running)                                        // 进入运行态
                        _animationTimer->start();                       // 启动虚线动画
                    else                                                // 回到停止态
                        _animationTimer->stop();                        // 停止动画
                }
                refreshConnections();                                   // 立即重绘一次连线
            });
}

// 动画定时心跳：推进虚线偏移并重绘所有连线
void MainWindow::onAnimationTick()
{
    if (!_connectionPainter)                    // 绘制器尚未就绪
        return;                                 // 忽略

    _connectionPainter->advancePhase(1.0);      // 推进虚线偏移量
    refreshConnections();                       // 重绘连线，形成流动效果
}

// 重绘场景中的所有连线（只刷新连线，不重绘节点）
void MainWindow::refreshConnections()
{
    if (!_scene)                                // 场景尚未就绪
        return;                                 // 忽略

    for (QGraphicsItem *item : _scene->items()) {                    // 遍历场景图元
        if (item->type() == QtNodes::ConnectionGraphicsObject::Type) // 只挑连线图元
            item->update();                                          // 请求重绘该连线
    }
}

// 初始化「串口」菜单：连接 / 断开 两个动作
void MainWindow::setupSerialMenu()
{
    QMenu *serialMenu = ui->menubar->addMenu(QStringLiteral("串口"));    // 顶部“串口”菜单
    _connectSerialAction = serialMenu->addAction(QStringLiteral("连接串口…")); // 连接动作
    _disconnectSerialAction = serialMenu->addAction(QStringLiteral("断开串口")); // 断开动作
    _disconnectSerialAction->setEnabled(false);                          // 初始为未连接态

    connect(_connectSerialAction, &QAction::triggered, this, &MainWindow::onConnectSerial);       // 连接串口
    connect(_disconnectSerialAction, &QAction::triggered, this, &MainWindow::onDisconnectSerial); // 断开串口

    // 串口开关状态变化时，刷新两个动作的可用性与文字
    connect(&SerialPortManager::instance(), &SerialPortManager::connectionChanged, this,
            [this](bool open) {
                _connectSerialAction->setEnabled(!open);                 // 已连接则禁用“连接”
                _disconnectSerialAction->setEnabled(open);               // 未连接则禁用“断开”
                _disconnectSerialAction->setText(                        // 断开的菜单文字
                    open ? QStringLiteral("断开串口（%1）")               // 已连接：附带串口名
                             .arg(SerialPortManager::instance().portName())
                         : QStringLiteral("断开串口"));                  // 未连接：普通文字
            });

    // 通信日志输出到调试控制台
    connect(&SerialPortManager::instance(), &SerialPortManager::logMessage, this,
            [](QString const &text) { qInfo().noquote() << text; });
}

// 菜单：弹出对话框选择并连接串口
void MainWindow::onConnectSerial()
{
    QDialog dialog(this);                                                // 串口选择对话框
    dialog.setWindowTitle(QStringLiteral("连接串口"));                    // 对话框标题

    auto *portCombo = new QComboBox(&dialog);                            // 串口下拉框
    auto *refreshButton = new QPushButton(QStringLiteral("刷新"), &dialog); // 刷新按钮
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                         &dialog);                       // 确定 / 取消

    auto fillPorts = [portCombo]() {                                     // 扫描并填充串口列表
        portCombo->clear();                                              // 清空旧列表
        for (QSerialPortInfo const &info : QSerialPortInfo::availablePorts()) // 遍历系统串口
            portCombo->addItem(info.portName(), info.portName());        // 显示名与数据都是串口名
    };
    fillPorts();                                                         // 首次填充

    connect(refreshButton, &QPushButton::clicked, &dialog, fillPorts);    // 点刷新重新扫描
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept); // 确定
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject); // 取消

    auto *row = new QHBoxLayout;                                         // 下拉框与刷新按钮一行
    row->addWidget(portCombo);                                           // 串口下拉框
    row->addWidget(refreshButton);                                       // 刷新按钮
    auto *layout = new QVBoxLayout(&dialog);                             // 对话框纵向布局
    layout->addLayout(row);                                              // 第一行
    layout->addWidget(buttons);                                          // 按钮行

    if (dialog.exec() != QDialog::Accepted)                              // 用户取消
        return;                                                          // 直接返回

    QString const portName = portCombo->currentData().toString();         // 选中的串口名
    if (portName.isEmpty()) {                                            // 没有可选串口
        QMessageBox::warning(this, QStringLiteral("连接串口"),
                             QStringLiteral("未找到可用串口，请检查设备连接")); // 提示
        return;                                                          // 直接返回
    }

    bool ok = false;                                                     // 打开结果
    QMetaObject::invokeMethod(&SerialPortManager::instance(), [&ok, portName]() {
        ok = SerialPortManager::instance().open(portName);              // 在工作线程里打开串口
    }, Qt::BlockingQueuedConnection);                                    // 阻塞等待结果
    if (!ok)                                                             // 打开失败
        QMessageBox::warning(this, QStringLiteral("连接串口"),
                             QStringLiteral("无法打开串口 %1").arg(portName)); // 提示失败原因
}

// 菜单：断开串口
void MainWindow::onDisconnectSerial()
{
    QMetaObject::invokeMethod(&SerialPortManager::instance(), []() {
        SerialPortManager::instance().close();  // 在工作线程里关闭串口
    }, Qt::QueuedConnection);                   // 异步，不阻塞 UI
}

// 菜单：进入运行态
void MainWindow::onRun()
{
    if (_graphModel)                            // 判空
        _graphModel->start();                   // 切到运行态并触发一次数据传递
}

// 菜单：回到停止态
void MainWindow::onStop()
{
    if (_graphModel)                            // 判空
        _graphModel->stop();                    // 切到停止态，之后的投递会被丢弃
}

// 节点右键菜单：弹出并处理“删除节点”
void MainWindow::onNodeContextMenu(QtNodes::NodeId nodeId, QPointF const &scenePos)
{
    QMenu menu(this);                                                    // 创建右键菜单
    QAction *deleteAction = menu.addAction(QStringLiteral("删除节点"));   // 添加删除项
    // 信号给的是场景坐标，菜单需要屏幕全局坐标，这里做一次转换
    QPoint const globalPos = _view ? _view->mapToGlobal(_view->mapFromScene(scenePos))
                                   : QCursor::pos();                     // 兜底用鼠标位置
    QAction *selected = menu.exec(globalPos);                            // 在点击处弹出菜单
    if (selected == deleteAction && _graphModel)                         // 判断是否点了删除
        _graphModel->deleteNode(nodeId);                                 // 删除该节点
}