//
// Created by fazhehy on 2023/1/26.
//

#include "PIDWidget.h"
#include <QLabel>

#include <QFormLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QPainter>
#include <QWheelEvent>
#include <QGraphicsGridLayout>
#include <QShortcut>

#include <QDebug>
#include <QMessageBox>

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
    scrollBar = new QScrollBar;
    chartView = new QChartView;
    valueBrowser = new QTextBrowser;
    maxLabel = new QLabel;
    minLabel = new QLabel;

    this->init();
    chartView->installEventFilter(this);
    connect(serial, &Serial::analysisOver, this, [=](){
        updateCurve();
    });

    connect(sendButton, &QPushButton::clicked, this, [=](){
        if(serial->getSerialPort()->isOpen())
        {
            updateData();
            stopButton->setText("暂停");
            disableZoom = true;
        }
        else
            QMessageBox::warning(this, "警告", "串口已关闭！");
    });

    auto * shortcut = new QShortcut(this);
    shortcut->setKey(Qt::Key_Return);
    shortcut->setAutoRepeat(false);

    connect(shortcut, &QShortcut::activated, this, [=](){
        if(serial->getSerialPort()->isOpen())
        {
            updateData();
            stopButton->setText("暂停");
            disableZoom = true;
        }
        else
            QMessageBox::warning(this, "警告", "串口已关闭！");
    });

    connect(stopButton, &QPushButton::clicked, this, [=](){

        if(serial->getSerialPort()->isOpen())
        {
            if (stopButton->text() == "暂停")
            {
                stopButton->setText("开始");
                serial->writeFrameData().setCommandMode();
                serial->writeFrameData() << char(0x02);
                serial->sendFrame();
                serial->writeFrameData().resetCommandMode();
                disableZoom = false;
            }
            else
            {
                stopButton->setText("暂停");
                serial->writeFrameData().setCommandMode();
                serial->writeFrameData() << char(0x03);
                serial->sendFrame();
                serial->writeFrameData().resetCommandMode();
                disableZoom = true;
            }
        }
        else
            QMessageBox::warning(this, "警告", "串口已关闭！");
    });

    connect(clearButton, &QPushButton::clicked, this, [=](){
        targetLine->clear();
        valueLine->clear();
        axisX->setRange(0, 50);
        axisX->setMax(50);
        axisY->setRange(-1*1.25, 10*1.25);
        scrollBar->setMinimum(50);
        scrollBar->setPageStep(scrollBar->width()+50);
        scrollBar->setValue(50);
        max_value = 0;
        min_value = 0;
        maxLabel->setText(QString::number(max_value, 'g', 2));
        minLabel->setText(QString::number(min_value, 'g', 2));
        clearFlag = true;
    });

    connect(restartButton, &QPushButton::clicked, this, [=](){

        if(serial->getSerialPort()->isOpen())
        {
            serial->writeFrameData().setCommandMode();
            serial->writeFrameData() << char(0x01);
            serial->sendFrame();
            serial->writeFrameData().resetCommandMode();
            stopButton->setText("暂停");
            disableZoom = true;
        }
        else
            QMessageBox::warning(this, "警告", "串口已关闭！");
    });

    connect(scrollBar, &QScrollBar::sliderMoved, this, [=](){
       qDebug() << scrollBar->value();
       scrollBar->setRange(0, scrollBar->maximum());
       if (scrollBar->value() > 50)
           axisX->setRange(scrollBar->value()-50, scrollBar->value());
       else
           axisX->setRange(0, 50);
       rollFlag = false;
    });

    connect(valueBrowser, &QTextBrowser::textChanged, this,[=](){
       valueBrowser->moveCursor(QTextCursor::End);
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
    serial->writeFrameData() << (char)0x01;
    serial->sendFrame();
    serial->writeFrameData().resetCommandMode();
}

bool PIDWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this->chartView)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            rollFlag = !rollFlag;
            scrollBar->setMinimum(50);
            if (rollFlag)
            {
                axisX->setRange(scrollBar->maximum()-50, scrollBar->maximum());
                scrollBar->setSliderPosition(scrollBar->maximum());
                disableZoom = true;
            }
            else
                disableZoom = false;
            return true;
        }
        else if (event->type() == QEvent::Wheel)
        {
            auto * wheelEvent = dynamic_cast<QWheelEvent *>(event);
            if (wheelEvent->modifiers() == Qt::ControlModifier)
            {
                if (!disableZoom)
                {
                    qDebug() << wheelEvent->position().y();
                    qDebug() << chart->size().height();
                    double k = (wheelEvent->position().y()-50)/(chart->size().height()-73);
                    double loc = (axisY->max() - axisY->min())*(1-k)+axisY->min();
                    double h1 = loc-axisY->min();
                    double h2 = axisY->max()-loc;
                    if (wheelEvent->angleDelta().y() > 0)
                        axisY->setRange(loc-h1*0.75, loc+h2*0.75);
                    else
                        axisY->setRange(loc-h1*1.25, loc+h2*1.25);
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void PIDWidget::updateCurve()
{
    static QString data;
    serial->readFrameData().at(1, &target);
    serial->readFrameData().at(2, &value);
    data.append(QString::number(value, 'g', 2)+'\n');
    if (clearFlag)
    {
        data.clear();
        clearFlag = false;
    }
    valueBrowser->setPlainText(data);
    if (value >= max_value)
    {
        max_value = value;
        maxLabel->setText(QString::number(max_value, 'g', 2));
    }

    if (value <= min_value)
    {
        min_value = value;
        minLabel->setText(QString::number(min_value, 'g', 2));
    }

    float tmp_max = max_value>10?max_value:10;
    float tmp_min = min_value< -1?min_value: -1;
    if (tmp_max <= target)
        tmp_max = target;
    if (tmp_min >= target)
        tmp_min = target;
    if (disableZoom)
        axisY->setRange(tmp_min*1.25, tmp_max*1.25);
    int count = (int) valueLine->points().size();
    if (count >= 50)
        scrollBar->setRange(50, count);
    if (rollFlag)
    {
        if (count >= 50)
        {
            axisX->setRange(count-50, count);
            scrollBar->setSliderPosition(count);
        }

    }
    targetLine->append(QPointF(count, target));
    valueLine->append(QPointF(count, value));
}

void PIDWidget::init()
{
    auto * viewWidget = new QWidget(this);
    auto * editWidget = new QWidget(viewWidget);
    auto * buttonWidget = new QWidget(this);
    auto * chartWidget = new QWidget(viewWidget);
    chartView->setParent(chartWidget);
    valueBrowser->setParent(chartWidget);
    valueBrowser->setFixedHeight(50);

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

    auto *valueLabel = new QWidget(editWidget);
    auto * valueLabel_1 = new QWidget(valueLabel);
    auto * maxLabelTitle = new QLabel(valueLabel_1);
    maxLabelTitle->setFont(font);
    maxLabelTitle->setText("最大值");
    maxLabel->setParent(valueLabel_1);
    maxLabel->setFont(font);
    maxLabel->setText(QString::number(max_value, 'g', 2));

    auto * labelLayout_1 = new QVBoxLayout(valueLabel_1);
    labelLayout_1->addWidget(maxLabelTitle);
    labelLayout_1->addWidget(maxLabel);

    auto * valueLabel_2 = new QWidget(valueLabel);
    auto * minLabelTitle = new QLabel(valueLabel_2);
    minLabelTitle->setFont(font);
    minLabelTitle->setText("最小值");
    minLabel->setParent(valueLabel_2);
    minLabel->setFont(font);
    minLabel->setText(QString::number(min_value, 'g', 2));

    auto * labelLayout_2 = new QVBoxLayout(valueLabel_2);
    labelLayout_2->addWidget(minLabelTitle);
    labelLayout_2->addWidget(minLabel);

    auto * labelLayout = new QHBoxLayout(valueLabel);
    labelLayout->addWidget(valueLabel_1);
    labelLayout->addWidget(valueLabel_2);

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

    axisX->setMax(50);
//    axisX->setTickCount(50);
    axisX->setLabelFormat("%d");
    axisX->setMinorTickCount(5);
    axisX->setGridLineVisible(true);
    axisX->setMin(0);
//    axisY->setMax(AXIS_MAX_Y);
//    axisY->setTickCount(50);
    axisY->setMinorTickCount(5);
    axisY->setLabelFormat("%.2f");
    axisY->setGridLineVisible(true);
//    axisY->setMin(0);
    axisY->setRange((min_value< -1?min_value: -1)*1.25, (max_value>10?max_value:10)*1.25);

    valueLine->setPointsVisible(true);
    valueLine->setName("Value");

    targetLine->setPointsVisible(true);
    targetLine->setName("Target");
    targetLine->setColor(Qt::red);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    chart->addSeries(valueLine);
    chart->addSeries(targetLine);
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setMargins(QMargins(0, 0, 0, 0));
//    chart->setAnimationOptions(QChart::SeriesAnimations);

    valueLine->attachAxis(axisX);
    valueLine->attachAxis(axisY);
    targetLine->attachAxis(axisX);
    targetLine->attachAxis(axisY);

    chartView->setChart(chart);
    scrollBar->setOrientation(Qt::Horizontal);

    scrollBar->setMinimum(50);
    scrollBar->setPageStep(scrollBar->width()+50);

    auto * chartLayout = new QVBoxLayout(chartWidget);
    chartLayout->addWidget(chartView);
    chartLayout->addWidget(scrollBar);
    chartLayout->addWidget(valueBrowser);

//    chartView->setRenderHint(QPainter::Antialiasing);

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
    vLayout->addWidget(pidValueGroup);
    vLayout->addWidget(valueLabel);

//    auto * chartLayout = new QVBoxLayout(chartWidget);
//    chartLayout->addWidget(chartView);
//    chartLayout->addWidget(slider);

    auto * hLayout = new QHBoxLayout(viewWidget);
    hLayout->addWidget(editWidget);
    hLayout->addWidget(chartWidget);

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

void PIDWidget::reset()
{
    serial->reset();
    settingGroup->reset();
    serial->setProtocol("read{"
                        "float:2;"
                        "}"
                        "write{"
                        "float:4;"
                        "}");
}
