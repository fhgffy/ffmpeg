#ifndef FACEREGISTERDIALOG_H
#define FACEREGISTERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include "faceapimanager.h"

class FaceRegisterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FaceRegisterDialog(QWidget *parent = nullptr);

private slots:
    void onBrowseImage();
    void onRegister();

private:
    QLineEdit *m_ipEdit;
    QLineEdit *m_userEdit;
    QLineEdit *m_pwdEdit;
    QLabel *m_imgLabel;
    QImage m_currentImg;
    FaceApiManager *m_manager;
};

#endif // FACEREGISTERDIALOG_H
