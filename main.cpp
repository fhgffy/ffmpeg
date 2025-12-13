#include "mainwidget.h"
#include "LoginWidget.h"
#include <QApplication>
#include <QScreen>
#include <QStyleFactory>
#include <functional> // 引入 function

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));

    // 定義兩個函數指針，用於相互調用
    std::function<void()> showLogin;
    std::function<void()> showMain;

    // 1. 顯示登錄界面的邏輯
    showLogin = [&]() {
        LoginWidget *login = new LoginWidget;

        // 當登錄成功時 -> 顯示主界面
        QObject::connect(login, &LoginWidget::sigLoginSuccess, [&, login](){
            showMain();          // 進入主界面
            login->close();      // 關閉登錄窗口
            login->deleteLater();// 釋放內存
        });

        login->show();
    };

    // 2. 顯示主界面的邏輯
    showMain = [&]() {
        MainWidget *w = new MainWidget;

        // 設置窗口大小和位置 (復用原來的代碼)
        QScreen *screen = QApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->availableGeometry();
            int width = screenGeometry.width() * 0.8;
            int height = screenGeometry.height() * 0.8;
            w->resize(width, height);
            w->move((screenGeometry.width() - width) / 2,
                    (screenGeometry.height() - height) / 2);
        }

        // 當點擊退出登錄時 -> 顯示登錄界面
        QObject::connect(w, &MainWidget::sigLogout, [&, w](){
            showLogin();     // 回到登錄界面
            w->close();      // 關閉主界面
            w->deleteLater();// 釋放內存
        });

        w->show();
        // 確保關閉主窗口時程序能退出（但切換時不會退出，因為我們馬上開了新窗口）
        a.setQuitOnLastWindowClosed(true);
    };

    // 3. 程序啓動，先顯示登錄
    showLogin();

    return a.exec();
}
