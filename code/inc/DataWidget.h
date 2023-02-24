//
// Created by fazhehy on 2023/1/26.
//

#ifndef SERIALASSISTANT_DATAWIDGET_H
#define SERIALASSISTANT_DATAWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QTextBrowser>
#include "Serial.h"
#include "SettingGroup.h"

class DataWidget : public QWidget{
    Q_OBJECT
private:
    Serial * serial;
    SettingGroup * settingGroup;
    QPushButton * saveButton;
    QPushButton * clearButton;
    QGroupBox * showSettingGroup;
    QCheckBox * showHexCheckBox;
    QTextBrowser * textBrowser;
    QGroupBox * sendSettingGroup;
    QCheckBox * newLineBox;
    QCheckBox * hexBox;
    QSpinBox * cycleValueEdit;
    QTextEdit * textEdit;
    QPushButton * sendButton;
    QPushButton * clearSendButton;
    QPushButton * stopSendButton;
public:
    DataWidget(Serial * pSerial, QWidget * parent = nullptr);
    void init();
    void paintEvent(QPaintEvent * e);

    void getSerialData();
    void sendSerialData(QString & data_);
    void showInHex();

    void reset();
private:
    QTimer * timer = nullptr;
    QString data;
    bool hexShowFlag = false;
    bool sendHex = false;
    bool sendNewLine = false;
};


#endif //SERIALASSISTANT_DATAWIDGET_H
