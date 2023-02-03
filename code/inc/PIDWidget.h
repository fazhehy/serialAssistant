//
// Created by fazhehy on 2023/1/26.
//

#ifndef SERIALASSISTANT_PIDWIDGET_H
#define SERIALASSISTANT_PIDWIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QValueAxis>
#include <QLineSeries>
#include <QChart>
#include <QPushButton>
#include "Serial.h"
#include "SettingGroup.h"

class PIDWidget : public QWidget{
    Q_OBJECT
private:
    Serial * serial;
    SettingGroup * settingGroup;
    QGroupBox * pidValueGroup;
    QDoubleSpinBox * pValueEdit;
    QDoubleSpinBox * iValueEdit;
    QDoubleSpinBox * dValueEdit;
    QDoubleSpinBox * targetEdit;
    QValueAxis * axisX;
    QValueAxis * axisY;
    QLineSeries * valueLine;
    QLineSeries * targetLine;
    QChart * chart;
    QPushButton * sendButton;
    QPushButton * stopButton;
    QPushButton * clearButton;
    QPushButton * restartButton;
public:
    PIDWidget(Serial * pSerial, QWidget * parent = nullptr);
    void init();
    void paintEvent(QPaintEvent * e);

    void updateCurve();
    void updateData();
private:
    const int AXIS_MAX_Y = 15;
    const int AXIS_MAX_X = 10;
    float P = 0;
    float I = 0;
    float D = 0;
    float value = 0;
    float target = 0;
};


#endif //SERIALASSISTANT_PIDWIDGET_H
