#include "mainwidget.h"
#include "QScreen"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWidget w;
    // 获取屏幕大小，设置为屏幕的80%
        QScreen *screen = QApplication::primaryScreen();
        QRect screenGeometry = screen->availableGeometry();
        int width = screenGeometry.width() * 0.8;
        int height = screenGeometry.height() * 0.8;
        w.resize(width, height);

        // 居中显示
        w.move((screenGeometry.width() - width) / 2,
               (screenGeometry.height() - height) / 2);
    w.show();
    return a.exec();
}
