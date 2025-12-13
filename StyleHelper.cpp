#include "StyleHelper.h"  //样式助手类 (StyleHelper)
#include <QApplication>

QString StyleHelper::getAppStyleSheet()
{
    return QString(R"(
        /* 主窗口样式 */
        QWidget {
            background-color: %1;
            color: %2;
        }

        /* 标签样式 */
        QLabel {
            color: %2;
            font-size: 14px;
            font-weight: 500;
        }

        /* 输入框样式 */
        QLineEdit {
            background-color: rgba(255, 255, 255, 0.1);
            border: 2px solid %6;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            color: %2;
            selection-background-color: %3;
        }

        QLineEdit:focus {
            border-color: %3;
            background-color: rgba(255, 255, 255, 0.15);
        }

        QLineEdit::placeholder {
            color: rgba(255, 255, 255, 0.6);
        }

        /* 按钮样式 */
        QPushButton {
            background-color: %3;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 14px;
            font-size: 16px;
            font-weight: bold;
            min-width: 120px;
        }

        QPushButton:hover {
            background-color: %4;
            transform: translateY(-1px);
        }

        QPushButton:pressed {
            background-color: %5;
            transform: translateY(1px);
        }

        QPushButton:disabled {
            background-color: rgba(255, 255, 255, 0.2);
            color: rgba(255, 255, 255, 0.5);
        }

        /* 链接按钮样式 */
        QPushButton.link-button {
            background-color: transparent;
            color: %3;
            font-weight: normal;
            text-decoration: underline;
            min-width: auto;
        }

        /* 复选框样式 */
        QCheckBox {
            color: %2;
            font-size: 14px;
        }

        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid %6;
            border-radius: 4px;
        }

        QCheckBox::indicator:checked {
            background-color: %3;
            border-color: %3;
        }

        /* 分割线 */
        QFrame[frameShape="4"] {
            background-color: rgba(255, 255, 255, 0.2);
            max-height: 1px;
            min-height: 1px;
        }
    )")
    .arg(backgroundColor())
    .arg(textColor())
    .arg(accentColor())
    .arg(primaryColor())
    .arg(secondaryColor())
    .arg(borderColor());
}

QPalette StyleHelper::getAppPalette()
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(backgroundColor()));
    palette.setColor(QPalette::WindowText, QColor(textColor()));
    palette.setColor(QPalette::Base, QColor("#2D3748"));
    palette.setColor(QPalette::AlternateBase, QColor("#4A5568"));
    palette.setColor(QPalette::ToolTipBase, QColor("#F7FAFC"));
    palette.setColor(QPalette::ToolTipText, QColor("#1A202C"));
    palette.setColor(QPalette::Text, QColor(textColor()));
    palette.setColor(QPalette::Button, QColor(accentColor()));
    palette.setColor(QPalette::ButtonText, QColor("#FFFFFF"));
    palette.setColor(QPalette::BrightText, QColor("#FFFFFF"));
    palette.setColor(QPalette::Link, QColor("#4299E1"));
    palette.setColor(QPalette::Highlight, QColor(accentColor()));
    palette.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));
    return palette;
}

QFont StyleHelper::getTitleFont()
{
    QFont font("Arial", 28, QFont::Bold);
    return font;
}

QFont StyleHelper::getLabelFont()
{
    QFont font("Arial", 11, QFont::Medium);
    return font;
}

QFont StyleHelper::getInputFont()
{
    QFont font("Arial", 13);
    return font;
}

QFont StyleHelper::getButtonFont()
{
    QFont font("Arial", 14, QFont::DemiBold);
    return font;
}

QString StyleHelper::primaryColor()      { return "#2D3748"; }    // 深蓝灰
QString StyleHelper::secondaryColor()    { return "#4A5568"; }    // 中蓝灰
QString StyleHelper::accentColor()       { return "#4299E1"; }    // 亮蓝色
QString StyleHelper::backgroundColor()   { return "#1A202C"; }    // 深色背景
QString StyleHelper::textColor()         { return "#F7FAFC"; }    // 浅灰白
QString StyleHelper::borderColor()       { return "#4A5568"; }    // 边框颜色
