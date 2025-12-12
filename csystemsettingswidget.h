#ifndef CSYSTEMSETTINGSWIDGET_H
#define CSYSTEMSETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>

class QLineEdit;
class QComboBox;
class QPushButton;
class QCheckBox;
class QSlider;
class QSpinBox;
class QNetworkAccessManager;

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
    // 【新增】消息信号
    void sigMessage(const QString &msg);
private slots:
    void onSaveClicked();
    // 【新增】保存图像参数
    void onSaveImageParamsClicked();


private:
    void setupUi();
    void loadSettings(); // 加载配置
    void initLocalIps(); // 自动扫描本机IP填入下拉框
    // 【新增】发送API请求帮助函数
    void sendApiRequest(const QMap<QString, QString>& params);

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

    // 【新增】图像参数控件
    QCheckBox *m_chkManualExposure; // 手动曝光
    QSpinBox *m_spinExposureTime;   // 曝光时间
    QCheckBox *m_chkWhiteLight;     // 全彩模式
    QComboBox *m_comboLightControl; // 灯光控制模式
    QSlider *m_sliderLightDepress;  // 强光抑制
    QSlider *m_sliderBLC;           // 背光补偿
    QSlider *m_sliderBrightness;    // 亮度
    QSlider *m_sliderContrast;      // 对比度
    QSlider *m_sliderSaturation;    // 饱和度
    QSlider *m_sliderSharpness;     // 锐度

    QNetworkAccessManager *m_netManager;
};

#endif // CSYSTEMSETTINGSWIDGET_H
