#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QtCore/QByteArray>                   // 接收缓冲区
#include <QtCore/QObject>                      // 基类：QObject
#include <QtCore/QString>                      // 字符串

class QSerialPort;                             // 前置声明：串口对象

/// 串口管理器（全局单例）：负责与下位机（STM32F103C8T6）通信。
/// 协议见《上位机通信协议.md》：
///  - 下行：1 个字节命令，只有低 4 位有效，且是“全量状态”（为 0 的位会关闭对应外设）
///  - 上行：ASCII 文本行，以 \r\n 结束
/// 位定义：bit0 红灯 / bit1 蓝灯 / bit2 蜂鸣器 / bit3 请求回发温湿度（单次触发，不保持）
class SerialPortManager : public QObject
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    /// 取全局唯一实例
    static SerialPortManager &instance();

    /// 打开串口（默认 115200-8-N-1），成功返回 true
    bool open(QString const &portName, int baudRate = 115200);

    /// 关闭串口
    void close();

    /// 串口是否已打开
    bool isOpen() const;

    /// 当前串口名（未连接时为空串）
    QString portName() const;

    /// 设置某个外设位（bit0 红灯 / bit1 蓝灯 / bit2 蜂鸣器）并按“全量状态”下发
    void setOutputBit(int bit, bool on);

    /// 清空所有输出位并下发 0x00（全部关闭），用于从运行态转为停止态
    void resetOutputs();

    /// 请求一次温湿度并阻塞等待回复；成功时写回温度和湿度
    bool requestTemperatureHumidity(double &temperature, double &humidity, int timeoutMs = 500);

Q_SIGNALS:
    void connectionChanged(bool open);         // 串口开关状态变化
    void logMessage(QString const &text);      // 通信日志

private:
    explicit SerialPortManager(QObject *parent = nullptr); // 构造函数（单例，私有）

    void sendByte(quint8 value);               // 下发一个命令字节（只取低 4 位）
    bool readLine(QString &line, int timeoutMs); // 阻塞读取一行上行数据

    QSerialPort *_serial = nullptr;            // 串口对象
    QString _portName;                         // 当前串口名缓存（供其他线程安全读取）
    QByteArray _rxBuffer;                      // 接收缓冲区（跨多次读取保留残余字节）
    quint8 _stateMask = 0;                     // 持久状态位（bit0~bit2），bit3 不入此
};

#endif // SERIALPORTMANAGER_H
