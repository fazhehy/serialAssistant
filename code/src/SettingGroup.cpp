//
// Created by fazhehy on 2023/1/26.
//

#include "SettingGroup.h"
#include <QFont>
#include <QLabel>
#include <QFormLayout>
#include <QTimer>
#include <QMessageBox>

SettingGroup::SettingGroup(Serial *pSerial, QWidget *parent) : QGroupBox(parent), serial(pSerial)
{
    serialNameComBox = new QComboBox(this);
    baudComBox = new QComboBox(this);
    stopBitsComBox = new QComboBox(this);
    dataBitsComBox = new QComboBox(this);
    parityComBox = new QComboBox(this);
    serialStateButton = new QPushButton(this);

    this->init();

    void (QComboBox:: * comBoxSignal)(const QString &) = &QComboBox::currentTextChanged;

    auto * timer = new QTimer;
    connect(timer, &QTimer::timeout, this, [=](){
        updateSerialNameList();
        int index;
        if(!removeSerialName.isEmpty())
        {
            for(auto & i : removeSerialName)
            {
                index = serialNameComBox->findText(i);
                if(serialNameComBox->currentText() == i)
                {
                    serialNameComBox->setCurrentIndex(0);
                    serial->getSerialPort()->close();
                    serialStateButton->setText("关闭");
                }
                serialNameComBox->removeItem(index);
            }
        }
        if(!newSerialName.isEmpty())
        {
            for(auto & i : newSerialName)
                serialNameComBox->addItem(i);
        }
    });
    timer->start(500);
    connect(serialNameComBox, comBoxSignal, this, [=](){
        serial->getSerialPort()->setPortName(serialNameComBox->currentText());
    });

    connect(baudComBox, comBoxSignal, this, [=](){
        serial->getSerialPort()->setBaudRate(static_cast<QSerialPort::BaudRate>(baudComBox->currentText().toInt()));
    });

    connect(stopBitsComBox, comBoxSignal, this, [=](){
        QString tmp = stopBitsComBox->currentText();
        if(tmp == "1")
            serial->getSerialPort()->setStopBits(QSerialPort::OneStop);
        else if(tmp == "1.5")
            serial->getSerialPort()->setStopBits(QSerialPort::OneAndHalfStop);
        else if(tmp == "2")
            serial->getSerialPort()->setStopBits(QSerialPort::TwoStop);
    });

    connect(dataBitsComBox, comBoxSignal, this, [=](){
        serial->getSerialPort()->setDataBits(static_cast<QSerialPort::DataBits>(dataBitsComBox->currentText().toInt()));
    });

    connect(parityComBox, comBoxSignal, this, [=](){
        QString str = parityComBox->currentText();
        if(str == "None")
            serial->getSerialPort()->setParity(QSerialPort::NoParity);
        else if(str == "Odd")
            serial->getSerialPort()->setParity(QSerialPort::OddParity);
        else if(str == "Even")
            serial->getSerialPort()->setParity(QSerialPort::EvenParity);
    });

    connect(serialStateButton, &QPushButton::clicked, this, [=](){
        if(serialStateButton->text() == "关闭")
        {
            if(!serial->getSerialPort()->open(QIODeviceBase::ReadWrite))
            {
                QMessageBox::critical(this, "错误", "串口打开失败！");
                exit(-1);
            }
            else
                serialStateButton->setText("打开");
        }
        else if(serialStateButton->text() == "打开")
        {
            serial->getSerialPort()->close();
            serialStateButton->setText("关闭");
        }
    });
}

void SettingGroup::init()
{
    QFont font(QStringLiteral("微软雅黑"), 12);
    QString comboxStyle = QString::fromUtf8("QComboBox{\n"
                                            "  font-size:14px;\n"
                                            "  padding: 1px 15px 1px 3px;\n"
                                            "  border:1px solid rgba(228,228,228,1);\n"
                                            "  background-color: white;\n"
                                            "  border-radius:5px 5px 0px 0px;\n"
                                            "} \n"
                                            "  QComboBox::drop-down {\n"
                                            "      subcontrol-origin: padding;\n"
                                            "      subcontrol-position: top right;\n"
                                            "      width: 15px;\n"
                                            "      border:none;\n"
                                            "}");

    QString buttonStyle = QString::fromUtf8("QPushButton{\n"
                                            "  font-size:14px;\n"
                                            "  border:1px ;\n"
                                            "  font-weight: 570;\n"
                                            "  border:1px solid rgba(228,228,228,1);\n"
                                            "  border-radius:5px 5px 0px 0px;\n"
                                            "  background-color: white;\n"
                                            "}"
                                            "QPushButton:pressed{"
                                            "    background-color:rgb(14 , 135 , 228);"
                                            "    padding-left:3px;"
                                            "    padding-top:3px;"
                                            "}");

    this->setTitle("串口设置");

    auto * serialNameLabel = new QLabel(this);
    auto * baudLabel = new QLabel(this);
    auto * stopBitsLabel = new QLabel(this);
    auto * dataBitsLabel = new QLabel(this);
    auto * parityLabel = new QLabel(this);
    auto * serialStateLabel = new QLabel(this);

    serialNameLabel->setFont(font);
    serialNameLabel->setText("串口号：");
    baudLabel->setFont(font);
    baudLabel->setText("波特率：");
    stopBitsLabel->setFont(font);
    stopBitsLabel->setText("停止位：");
    dataBitsLabel->setFont(font);
    dataBitsLabel->setText("数据位：");
    parityLabel->setFont(font);
    parityLabel->setText("校验位：");
    serialStateLabel->setFont(font);
    serialStateLabel->setText("串口状态：");
    serialStateButton->setFont(font);
    serialStateButton->setText("关闭");

    serialNameComBox->setFont(font);
    serialNameComBox->setStyleSheet(comboxStyle);
    baudComBox->setFont(font);
    baudComBox->setStyleSheet(comboxStyle);
    stopBitsComBox->setFont(font);
    stopBitsComBox->setStyleSheet(comboxStyle);
    dataBitsComBox->setFont(font);
    dataBitsComBox->setStyleSheet(comboxStyle);
    parityComBox->setFont(font);
    parityComBox->setStyleSheet(comboxStyle);
    serialStateButton->setFont(font);
    serialStateButton->setStyleSheet(buttonStyle);

    serialNameList = Serial::getAvailableSerialName();
    serialNameComBox->addItems(serialNameList);

    baudComBox->addItem(QString::number(115200));
    baudComBox->addItem(QString::number(57600));
    for(int baud = 38400; baud >= 1200; baud /= 2)
        baudComBox->addItem(QString::number(baud));
    serial->getSerialPort()->setBaudRate(static_cast<QSerialPort::BaudRate>(baudComBox->currentText().toInt()));

    stopBitsComBox->addItem("1");
    stopBitsComBox->addItem("1.5");
    stopBitsComBox->addItem("2");
    serial->getSerialPort()->setStopBits(QSerialPort::OneStop);

    dataBitsComBox->addItem(QString::number(8));
    dataBitsComBox->addItem(QString::number(7));
    dataBitsComBox->addItem(QString::number(6));
    dataBitsComBox->addItem(QString::number(5));
    serial->getSerialPort()->setDataBits(static_cast<QSerialPort::DataBits>(dataBitsComBox->currentText().toInt()));

    parityComBox->addItem("None");
    parityComBox->addItem("Odd");
    parityComBox->addItem("Even");
    serial->getSerialPort()->setParity(QSerialPort::NoParity);

    auto * layout = new QFormLayout(this);
    layout->setWidget(0, QFormLayout::LabelRole, serialNameLabel);
    layout->setWidget(1, QFormLayout::LabelRole, baudLabel);
    layout->setWidget(2, QFormLayout::LabelRole, stopBitsLabel);
    layout->setWidget(3, QFormLayout::LabelRole, dataBitsLabel);
    layout->setWidget(4, QFormLayout::LabelRole, parityLabel);
    layout->setWidget(5, QFormLayout::LabelRole, serialStateLabel);

    layout->setWidget(0, QFormLayout::FieldRole, serialNameComBox);
    layout->setWidget(1, QFormLayout::FieldRole, baudComBox);
    layout->setWidget(2, QFormLayout::FieldRole, stopBitsComBox);
    layout->setWidget(3, QFormLayout::FieldRole, dataBitsComBox);
    layout->setWidget(4, QFormLayout::FieldRole, parityComBox);
    layout->setWidget(5, QFormLayout::FieldRole, serialStateButton);
}

void SettingGroup::updateSerialNameList()
{
    QStringList newSerialNameList;

    newSerialNameList += Serial::getAvailableSerialName();

    newSerialName.clear();
    removeSerialName.clear();
    if (serialNameList.size() != newSerialNameList.size())
    {
        for (auto & i : newSerialNameList)
        {
            if(serialNameList.indexOf(i) == -1)
                newSerialName.append(i);
        }
        for (auto & i : serialNameList)
        {
            if(newSerialNameList.indexOf(i) == -1)
                removeSerialName.append(i);
        }
        serialNameList = newSerialNameList;
    }
}

void SettingGroup::reset()
{
    serialStateButton->setText("关闭");
}
