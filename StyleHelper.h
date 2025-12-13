#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <QString>
#include <QPalette>
#include <QFont>

class StyleHelper
{
public:
    // 获取应用程序全局样式表
    static QString getAppStyleSheet();

    // 获取调色板
    static QPalette getAppPalette();

    // 获取字体
    static QFont getTitleFont();
    static QFont getLabelFont();
    static QFont getInputFont();
    static QFont getButtonFont();

    // 颜色常量
    static QString primaryColor();      // 主色调
    static QString secondaryColor();    // 辅助色
    static QString accentColor();       // 强调色
    static QString backgroundColor();   // 背景色
    static QString textColor();         // 文字颜色
    static QString borderColor();       // 边框颜色
};

#endif // STYLEHELPER_H
