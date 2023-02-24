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
#include <QScrollBar>
#include <QChartView>
#include <QTextBrowser>
#include <QLabel>

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
    QScrollBar * scrollBar;
    QChartView * chartView;
    QTextBrowser * valueBrowser;
    QLabel * maxLabel;
    QLabel * minLabel;
public:
    PIDWidget(Serial * pSerial, QWidget * parent = nullptr);
    void init();
    void paintEvent(QPaintEvent * e);
    bool eventFilter(QObject *obj, QEvent *event);

    void updateCurve();
    void updateData();
    void reset();
private:
//    int AXIS_MAX_Y = 0;
//    int AXIS_MAX_X = 50;
    float P = 0;
    float I = 0;
    float D = 0;
    float value = 0;
    float target = 0;
    float max_value = 0;
    float min_value = 0;
    bool rollFlag = true;
    bool clearFlag = false;
    bool disableZoom = true;
};


#endif //SERIALASSISTANT_PIDWIDGET_H
