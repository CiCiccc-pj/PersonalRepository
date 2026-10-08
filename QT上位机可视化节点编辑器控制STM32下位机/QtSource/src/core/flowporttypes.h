#ifndef FLOWPORTTYPES_H
#define FLOWPORTTYPES_H

#include <QtCore/QString>                      // 字符串
#include <QtNodes/NodeData>                    // NodeDataType

/// 流程编辑器用到的所有端口类型，集中在此定义，供各节点共用。
/// id 是内部比较用的稳定英文串，name 是界面上显示的中文名。
namespace FlowPortType {

// 六种端口类型的 id（内部比较用）
inline QString execId()   { return QStringLiteral("flow.exec"); }   // 执行流
inline QString anyId()    { return QStringLiteral("flow.any"); }    // 通用数据
inline QString intId()    { return QStringLiteral("flow.int"); }    // 整数
inline QString floatId()  { return QStringLiteral("flow.float"); }  // 浮点
inline QString boolId()   { return QStringLiteral("flow.bool"); }   // 布尔
inline QString stringId() { return QStringLiteral("flow.string"); } // 字符串

// 六种端口类型的完整定义（id + 中文显示名）
inline QtNodes::NodeDataType exec()   { return {execId(),   QStringLiteral("执行流")}; } // 执行流
inline QtNodes::NodeDataType any()    { return {anyId(),    QStringLiteral("通用")}; }   // 通用数据
inline QtNodes::NodeDataType integer(){ return {intId(),    QStringLiteral("整数")}; }   // 整数
inline QtNodes::NodeDataType real()   { return {floatId(),  QStringLiteral("浮点")}; }   // 浮点
inline QtNodes::NodeDataType boolean(){ return {boolId(),   QStringLiteral("布尔")}; }   // 布尔
inline QtNodes::NodeDataType text()   { return {stringId(), QStringLiteral("字符串")}; } // 字符串

} // namespace FlowPortType

#endif // FLOWPORTTYPES_H