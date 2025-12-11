#ifndef CSYSTEMSETTINGSWIDGET_H
#define CSYSTEMSETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>

class QLineEdit;
class QComboBox;
class QPushButton;

class CSystemSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CSystemSettingsWidget(QWidget *parent = nullptr);

    // 获取当前配置的公共接口
    QString getDeviceIp() const;
    QString getDeviceUser() const;
    QString getDevicePwd() const;
    QString getLocalIp() const;
    int getLocalPort() const;

signals:
    // 当点击“保存并应用”时发送此信号
    void sigConfigChanged();

private slots:
    void onSaveClicked();

private:
    void setupUi();
    void loadSettings(); // 加载配置
    void initLocalIps(); // 自动扫描本机IP填入下拉框

private:
    // 设备参数
    QLineEdit *m_deviceIpEdit;
    QLineEdit *m_deviceUserEdit;
    QLineEdit *m_devicePwdEdit;

    // 回调参数
    QComboBox *m_localIpCombo;
    QLineEdit *m_localPortEdit;

    // 持久化设置
    QSettings *m_settings;
};

#endif // CSYSTEMSETTINGSWIDGET_H
