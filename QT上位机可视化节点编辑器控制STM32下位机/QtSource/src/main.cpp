#include "mainwindow.h"                        // 主窗口

#include <QApplication>                        // Qt 应用类

// 程序入口
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);                // 创建应用对象
    MainWindow w;                              // 创建主窗口
    w.show();                                  // 显示主窗口
    return QApplication::exec();               // 进入事件循环
}