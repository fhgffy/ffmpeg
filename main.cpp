#include "mainwidget.h"
#include "LoginWidget.h"  // 引入登录窗口
#include <QApplication>
#include <QScreen>
#include <QStyleFactory>  // 用于设置Fusion样式

int main(int argc, char *argv[])
{
    // 1. 启用高分屏支持 (防止界面模糊)
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    #endif

    QApplication a(argc, argv);

    // 2. 设置 Fusion 样式，这对登录界面的圆角和颜色很重要
    a.setStyle(QStyleFactory::create("Fusion"));

    // 3. 创建登录窗口
    LoginWidget *login = new LoginWidget;

    // 4. 连接登录成功信号 -> 启动主窗口
    QObject::connect(login, &LoginWidget::sigLoginSuccess, [&](){

        // --- A. 创建并设置主监控窗口 ---
        MainWidget *w = new MainWidget;

        // 获取屏幕大小，设置为主窗口占 80%
        QScreen *screen = QApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->availableGeometry();
            int width = screenGeometry.width() * 0.8;
            int height = screenGeometry.height() * 0.8;
            w->resize(width, height);
            w->move((screenGeometry.width() - width) / 2,
                    (screenGeometry.height() - height) / 2);
        }

        // --- B. 显示主窗口 ---
        w->show();

        // --- C. 恢复应用程序退出机制 ---
        // (一旦主窗口打开，我们希望最后一个窗口关闭时程序退出)
        a.setQuitOnLastWindowClosed(true);

        // --- D. 销毁登录窗口 ---
        login->deleteLater();
    });

    // 5. 显示登录窗口
    login->show();

    // 6. 临时设置：防止登录窗口关闭（切换瞬间）导致程序退出
    a.setQuitOnLastWindowClosed(false);

    return a.exec();
}
