#include "faceregisterdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

FaceRegisterDialog::FaceRegisterDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("人脸注册");
    resize(400, 500);
    m_manager = new FaceApiManager(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 图片预览
    m_imgLabel = new QLabel("点击选择图片", this);
    m_imgLabel->setAlignment(Qt::AlignCenter);
    m_imgLabel->setFixedSize(200, 200);
    m_imgLabel->setStyleSheet("border: 2px dashed #666;");
    mainLayout->addWidget(m_imgLabel, 0, Qt::AlignCenter);

    QPushButton *btnBrowse = new QPushButton("选择图片", this);
    connect(btnBrowse, &QPushButton::clicked, this, &FaceRegisterDialog::onBrowseImage);
    mainLayout->addWidget(btnBrowse);

    // 表单
    QFormLayout *form = new QFormLayout;
    m_ipEdit = new QLineEdit("192.168.6.100"); // 默认摄像头IP
    m_userEdit = new QLineEdit("admin");
    m_pwdEdit = new QLineEdit("admin");
    m_pwdEdit->setEchoMode(QLineEdit::Password);

    form->addRow("摄像头IP:", m_ipEdit);
    form->addRow("用户名:", m_userEdit);
    form->addRow("密码:", m_pwdEdit);
    mainLayout->addLayout(form);

    // 按钮
    QPushButton *btnReg = new QPushButton("注册人脸", this);
    connect(btnReg, &QPushButton::clicked, this, &FaceRegisterDialog::onRegister);
    mainLayout->addWidget(btnReg);

    connect(m_manager, &FaceApiManager::sigRegisterResult, this, [=](bool ok, QString msg){
        if(ok) QMessageBox::information(this, "提示", msg);
        else QMessageBox::warning(this, "错误", msg);
    });
}

void FaceRegisterDialog::onBrowseImage() {
    QString path = QFileDialog::getOpenFileName(this, "选择人脸", "", "Images (*.png *.jpg)");
    if(!path.isEmpty()) {
        m_currentImg.load(path);
        m_imgLabel->setPixmap(QPixmap::fromImage(m_currentImg).scaled(m_imgLabel->size(), Qt::KeepAspectRatio));
    }
}

void FaceRegisterDialog::onRegister() {
    if(m_currentImg.isNull()) return;
    m_manager->registerFace(m_ipEdit->text(), m_userEdit->text(), m_pwdEdit->text(), m_currentImg);
}
