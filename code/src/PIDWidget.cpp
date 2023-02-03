//
// Created by fazhehy on 2023/1/26.
//

#include "PIDWidget.h"
#include <QLabel>
#include <QChartView>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QPainter>

#include <QDebug>

PIDWidget::PIDWidget(Serial *pSerial, QWidget *parent) : QWidget(parent), serial(pSerial)
{
    settingGroup = new SettingGroup(serial, this);

    pidValueGroup = new QGroupBox;
    pValueEdit = new QDoubleSpinBox;
    iValueEdit = new QDoubleSpinBox;
    dValueEdit = new QDoubleSpinBox;
    targetEdit = new QDoubleSpinBox;
    axisX = new QValueAxis;
    axisY = new QValueAxis;
    valueLine = new QLineSeries;
    targetLine = new QLineSeries;
    chart = new QChart;
    sendButton = new QPushButton;
    stopButton = new QPushButton;
    clearButton = new QPushButton;
    restartButton = new QPushButton;

    this->init();

    serial->setProtocol("read{"
                        "float:2;"
                        "}"
                        "write{"
                        "float:4;"
                        "}");

    connect(serial, &Serial::analysisOver, this, [=](){
        updateCurve();
    });

    connect(sendButton, &QPushButton::clicked, this, [=](){
        updateData();
    });

    connect(stopButton, &QPushButton::clicked, this, [=](){
        serial->writeFrameData().setCommandMode();
        serial->writeFrameData() << char(0x01);
        qDebug() << serial->writeFrameData().data().toHex(' ');
        serial->sendFrame();
        serial->writeFrameData().resetCommandMode();
    });

    connect(clearButton, &QPushButton::clicked, this, [=](){
        targetLine->clear();
        valueLine->clear();
        axisX->setRange(0, AXIS_MAX_X);
        axisY->setRange(0, AXIS_MAX_Y);
    });

    connect(restartButton, &QPushButton::clicked, this, [=](){
        serial->writeFrameData().setCommandMode();
        serial->writeFrameData() << '\0';
        serial->sendFrame();
        serial->writeFrameData().resetCommandMode();
    });
}

void PIDWidget::updateData()
{
    P = (float)pValueEdit->value();
    I = (float)iValueEdit->value();
    D = (float)dValueEdit->value();
    target = (float)targetEdit->value();
    serial->writeFrameData() << target << P << I << D;
    serial->sendFrame();
    serial->writeFrameData().setCommandMode();
    serial->writeFrameData() << '\0';
    serial->sendFrame();
    serial->writeFrameData().resetCommandMode();
}

void PIDWidget::updateCurve()
{
    serial->readFrameData().at(1, &target);
    serial->readFrameData().at(2, &value);
    int count = (int) valueLine->points().size();
    if (count >= AXIS_MAX_X)
        axisX->setMax(count);
    targetLine->append(QPointF(count, target));
    valueLine->append(QPointF(count, value));
}

void PIDWidget::init()
{
    auto * viewWidget = new QWidget(this);
    auto * editWidget = new QWidget(viewWidget);
    auto * buttonWidget = new QWidget(this);
    auto * chartView = new QChartView(viewWidget);

    auto * pValueLabel = new QLabel(pidValueGroup);
    auto * iValueLabel = new QLabel(pidValueGroup);
    auto * dValueLabel = new QLabel(pidValueGroup);
    auto * targetLabel = new QLabel(pidValueGroup);

    QFont font(QStringLiteral("微软雅黑"), 10);
    QString buttonStyle = QString::fromUtf8("QPushButton{\n"
                                            "    font-size:14px;\n"
                                            "    border:1px ;\n"
                                            "    font-weight: 570;\n"
                                            "    border:1px solid rgba(228,228,228,1);\n"
                                            "    border-radius:5px 5px 0px 0px;\n"
                                            "    background-color: #F6F7F8;\n"
                                            "}"
                                            "QPushButton:pressed{"
                                            "    background-color:rgb(14 , 135 , 228);"
                                            "    padding-left:3px;"
                                            "    padding-top:3px;"
                                            "}");

    QString valueEditStyle = QString::fromUtf8("QDoubleSpinBox{\n"
                                              "     border-radius:5px 5px 0px 0px;\n"
                                              "     border: 1px solid black;\n"
                                              "     font-size:14px;\n"
                                              "}");

    editWidget->setFixedWidth(215);

    settingGroup->setParent(editWidget);

    pValueEdit->setParent(pidValueGroup);
    pValueLabel->setFont(font);
    pValueLabel->setText("P");
    pValueEdit->setFont(font);
    pValueEdit->setStyleSheet(valueEditStyle);
    pValueEdit->setFixedHeight(50);
    pValueEdit->setSingleStep(0.5);
    pValueEdit->setRange(-10000, 10000);

    iValueEdit->setParent(pidValueGroup);
    iValueLabel->setFont(font);
    iValueLabel->setText("I");
    iValueEdit->setFont(font);
    iValueEdit->setStyleSheet(valueEditStyle);
    iValueEdit->setFixedHeight(50);
    iValueEdit->setSingleStep(0.5);
    iValueEdit->setRange(-10000, 10000);

    dValueEdit->setParent(pidValueGroup);
    dValueLabel->setFont(font);
    dValueLabel->setText("D");
    dValueEdit->setFont(font);
    dValueEdit->setStyleSheet(valueEditStyle);
    dValueEdit->setFixedHeight(50);
    dValueEdit->setSingleStep(0.5);
    dValueEdit->setRange(-10000, 10000);

    targetEdit->setParent(pidValueGroup);
    targetLabel->setFont(font);
    targetLabel->setText("目标");
    targetEdit->setFont(font);
    targetEdit->setStyleSheet(valueEditStyle);
    targetEdit->setFixedHeight(50);
    targetEdit->setSingleStep(0.5);
    targetEdit->setRange(-10000, 10000);

    sendButton->setParent(buttonWidget);
    sendButton->setStyleSheet(buttonStyle);
    sendButton->setFont(font);
    sendButton->setText("Send");

    stopButton->setParent(buttonWidget);
    stopButton->setStyleSheet(buttonStyle);
    stopButton->setFont(font);
    stopButton->setText("暂停");

    clearButton->setParent(buttonWidget);
    clearButton->setStyleSheet(buttonStyle);
    clearButton->setFont(font);
    clearButton->setText("清除");

    restartButton->setParent(buttonWidget);
    restartButton->setStyleSheet(buttonStyle);
    restartButton->setFont(font);
    restartButton->setText("重启");

    axisX->setMax(AXIS_MAX_X);
    axisX->setMin(0);
    axisY->setMax(AXIS_MAX_Y);
    axisY->setMin(0);

    valueLine->setPointsVisible(true);
    valueLine->setName("Value");

    targetLine->setPointsVisible(true);
    targetLine->setName("Target");
    targetLine->setColor(Qt::red);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    chart->addSeries(valueLine);
    chart->addSeries(targetLine);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    valueLine->attachAxis(axisX);
    valueLine->attachAxis(axisY);
    targetLine->attachAxis(axisX);
    targetLine->attachAxis(axisY);

    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    auto * groupFormLayout = new QFormLayout(pidValueGroup);

    groupFormLayout->setWidget(0, QFormLayout::LabelRole, pValueLabel);
    groupFormLayout->setWidget(0, QFormLayout::FieldRole, pValueEdit);
    groupFormLayout->setWidget(1, QFormLayout::LabelRole, iValueLabel);
    groupFormLayout->setWidget(1, QFormLayout::FieldRole, iValueEdit);
    groupFormLayout->setWidget(2, QFormLayout::LabelRole, dValueLabel);
    groupFormLayout->setWidget(2, QFormLayout::FieldRole, dValueEdit);
    groupFormLayout->setWidget(3, QFormLayout::LabelRole, targetLabel);
    groupFormLayout->setWidget(3, QFormLayout::FieldRole, targetEdit);

    auto * vLayout = new QVBoxLayout(editWidget);
    vLayout->addWidget(settingGroup);
    vLayout->addSpacing(50);
    vLayout->addWidget(pidValueGroup);

    auto * hLayout = new QHBoxLayout(viewWidget);
    hLayout->addWidget(editWidget);
    hLayout->addWidget(chartView);

    auto * buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(sendButton);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(restartButton);
    buttonLayout->addSpacing(10);

    auto * lastLayout = new QVBoxLayout(this);
    lastLayout->addWidget(viewWidget);
    lastLayout->addWidget(buttonWidget);
}

void PIDWidget::paintEvent(QPaintEvent * e)
{
    QPainter painter(this);

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRect(this->rect());
}
