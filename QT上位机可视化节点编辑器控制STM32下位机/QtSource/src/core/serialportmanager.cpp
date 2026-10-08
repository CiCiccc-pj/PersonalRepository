#include "serialportmanager.h"                 // 本类头文件

#include <QtCore/QElapsedTimer>                // 计时（超时控制）
#include <QtCore/QStringList>                  // 按分隔符拆分行
#include <QtCore/QThread>                      // 线程休眠（命令间隔）
#include <QtSerialPort/QSerialPort>            // 串口
#include <QtSerialPort/QSerialPortInfo>        // 串口信息

namespace {
int const kCommandIntervalMs = 20;             // 相邻命令字节的最小间隔（协议要求 ≥ 20ms）
quint8 const kRequestMask = 0x08;              // bit3：请求回发温湿度
quint8 const kCmdMask = 0x0F;                  // 命令字节只有低 4 位有效
} // namespace

// 取全局唯一实例（首次调用时构造）
SerialPortManager &SerialPortManager::instance()
{
    static SerialPortManager manager;           // 函数内静态对象，进程内唯一
    return manager;                             // 返回引用
}

// 构造函数：创建串口对象
SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)                           // 构造基类
    , _serial(new QSerialPort(this))             // 串口对象随本管理器释放
{}

// 打开串口：按协议配置 115200-8-N-1
bool SerialPortManager::open(QString const &portName, int baudRate)
{
    close();                                    // 先关闭已打开的串口

    _serial->setPortName(portName);             // 串口名
    _serial->setBaudRate(baudRate);             // 波特率
    _serial->setDataBits(QSerialPort::Data8);   // 数据位 8
    _serial->setParity(QSerialPort::NoParity);  // 无校验
    _serial->setStopBits(QSerialPort::OneStop); // 停止位 1
    _serial->setFlowControl(QSerialPort::NoFlowControl); // 无流控

    if (!_serial->open(QIODevice::ReadWrite)) { // 打开失败
        Q_EMIT logMessage(QStringLiteral("串口打开失败：%1（%2）")
                              .arg(portName, _serial->errorString())); // 记录原因
        return false;                           // 返回失败
    }

    _stateMask = 0;                             // 复位持久状态位
    _rxBuffer.clear();                          // 清空接收缓冲区
    _portName = portName;                       // 缓存串口名（供其他线程读取）
    Q_EMIT connectionChanged(true);             // 通知外部：已连接
    Q_EMIT logMessage(QStringLiteral("串口已打开：%1 @ %2").arg(portName).arg(baudRate)); // 记录日志
    return true;                                // 返回成功
}

// 关闭串口
void SerialPortManager::close()
{
    if (!_serial->isOpen())                     // 本来就未打开
        return;                                 // 无需处理

    _serial->close();                           // 关闭串口
    _rxBuffer.clear();                          // 清空接收缓冲区
    _stateMask = 0;                             // 复位状态位
    _portName.clear();                          // 清空串口名缓存
    Q_EMIT connectionChanged(false);            // 通知外部：已断开
    Q_EMIT logMessage(QStringLiteral("串口已关闭")); // 记录日志
}

// 串口是否已打开
bool SerialPortManager::isOpen() const
{
    return _serial->isOpen();                   // 直接问串口对象
}

// 当前串口名
QString SerialPortManager::portName() const
{
    return _portName;                           // 返回缓存（避免跨线程读 QSerialPort）
}

// 设置某个外设位并按“全量状态”下发
void SerialPortManager::setOutputBit(int bit, bool on)
{
    quint8 const mask = quint8(1u << bit);      // 该外设对应的位
    if (on)                                     // 要打开
        _stateMask |= mask;                     // 置 1
    else                                        // 要关闭
        _stateMask &= quint8(~mask);            // 清 0

    sendByte(_stateMask);                       // 全量下发当前状态
}

// 清空所有输出位并下发 0x00（全部关闭）
void SerialPortManager::resetOutputs()
{
    if (!_serial->isOpen())                     // 串口未打开
        return;                                 // 无处下发，直接忽略

    _stateMask = 0;                             // 复位持久状态位
    sendByte(0x00);                             // 下发“全部关闭”
}

// 请求一次温湿度并阻塞等待回复
bool SerialPortManager::requestTemperatureHumidity(double &temperature,
                                                   double &humidity,
                                                   int timeoutMs)
{
    temperature = 0.0;                          // 预设温度
    humidity = 0.0;                             // 预设湿度

    if (!_serial->isOpen()) {                   // 串口未打开
        Q_EMIT logMessage(QStringLiteral("温湿度读取失败：串口未打开")); // 记录日志
        return false;                           // 返回失败
    }

    sendByte(_stateMask | kRequestMask);        // 带上 bit3 请求回发（不改变持久状态）

    QElapsedTimer timer;                        // 超时计时器
    timer.start();                              // 开始计时

    while (timer.elapsed() < timeoutMs) {       // 在超时窗口内尝试读行
        QString line;                           // 收到的一行
        int const remain = timeoutMs - int(timer.elapsed()); // 剩余等待时间
        if (!readLine(line, remain))            // 读超时
            break;                              // 退出循环

        if (line.isEmpty())                     // 空行
            continue;                           // 跳过
        if (line.startsWith(QStringLiteral("DHT11 init"))) // 设备启动提示
            continue;                           // 不是数据行，跳过

        QStringList const fields = line.split(QLatin1Char(',')); // 形如 "T=26,H=59"
        if (fields.size() != 2)                 // 字段数不对
            continue;                           // 不是数据行，跳过

        QString const tempText = fields.at(0).split(QLatin1Char('=')).value(1); // "26" 或 "ERR"
        QString const humiText = fields.at(1).split(QLatin1Char('=')).value(1); // "59" 或 "ERR"
        if (tempText.isEmpty() || humiText.isEmpty()) // 取值失败
            continue;                           // 跳过该行

        if (tempText == QStringLiteral("ERR") || humiText == QStringLiteral("ERR")) { // 传感器读取失败
            Q_EMIT logMessage(QStringLiteral("温湿度读取失败（传感器无应答），可稍后重试")); // 记录日志
            return false;                       // 返回失败
        }

        temperature = tempText.toDouble();      // 写回温度
        humidity = humiText.toDouble();         // 写回湿度
        Q_EMIT logMessage(QStringLiteral("温湿度：%1℃ / %2%RH").arg(temperature).arg(humidity)); // 记录日志
        return true;                            // 返回成功
    }

    Q_EMIT logMessage(QStringLiteral("温湿度读取失败：等待回复超时")); // 记录超时
    return false;                               // 返回失败
}

// 下发一个命令字节（只取低 4 位）
void SerialPortManager::sendByte(quint8 value)
{
    if (!_serial->isOpen()) {                   // 串口未打开
        Q_EMIT logMessage(QStringLiteral("命令未发送：串口未打开")); // 记录日志
        return;                                 // 直接忽略
    }

    QByteArray const frame(1, char(value & kCmdMask)); // 只保留低 4 位
    _serial->write(frame);                      // 写入串口
    _serial->waitForBytesWritten(100);          // 等待写完成
    QString const hex = QStringLiteral("%1").arg(int(value & kCmdMask), 2, 16, QLatin1Char('0')).toUpper(); // 两位十六进制
    Q_EMIT logMessage(QStringLiteral("发送命令：0x%1").arg(hex)); // 记录日志
    QThread::msleep(kCommandIntervalMs);        // 协议要求相邻命令 ≥ 20ms，避免字节粘连
}

// 阻塞读取一行上行数据（以 \n 结束）
bool SerialPortManager::readLine(QString &line, int timeoutMs)
{
    QElapsedTimer timer;                        // 超时计时器
    timer.start();                              // 开始计时

    while (true) {                              // 循环直到读出整行或超时
        int const pos = _rxBuffer.indexOf('\n'); // 缓冲区里是否已有换行
        if (pos >= 0) {                         // 已收到一整行
            QByteArray const raw = _rxBuffer.left(pos); // 取换行之前的字节
            _rxBuffer.remove(0, pos + 1);       // 从缓冲区移除该行
            line = QString::fromLatin1(raw).trimmed(); // 去掉 \r 等空白
            return true;                        // 返回成功
        }

        int const remain = timeoutMs - int(timer.elapsed()); // 剩余等待时间
        if (remain <= 0)                        // 已超时
            return false;                       // 返回失败
        if (!_serial->waitForReadyRead(remain)) // 阻塞等待新数据
            return false;                       // 超时

        _rxBuffer += _serial->readAll();        // 追加到接收缓冲区
    }
}
