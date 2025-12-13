#include "mainwidget.h"
#include "LoginWidget.h"
#include <QApplication>
#include <QScreen>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    // 移除了 AA_EnableHighDpiScaling，解决比例和点击问题
    QApplication a(argc, argv);

    // 保持 Fusion 样式，让登录界面好看
    a.setStyle(QStyleFactory::create("Fusion"));

    LoginWidget *login = new LoginWidget;

    QObject::connect(login, &LoginWidget::sigLoginSuccess, [&](){
        MainWidget *w = new MainWidget;

        QScreen *screen = QApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->availableGeometry();
            int width = screenGeometry.width() * 0.8;
            int height = screenGeometry.height() * 0.8;
            w->resize(width, height);
            w->move((screenGeometry.width() - width) / 2,
                    (screenGeometry.height() - height) / 2);
        }

        w->show();
        a.setQuitOnLastWindowClosed(true);
        login->deleteLater();
    });

    login->show();
    a.setQuitOnLastWindowClosed(false);

    return a.exec();
}
