//
// Created by fazhehy on 2023/1/26.
//

#include "DataWidget.h"
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPainter>
#include <QFont>

DataWidget::DataWidget(Serial *pSerial, QWidget *parent) : QWidget(parent), serial(pSerial)
{
    settingGroup = new SettingGroup(serial, this);
    saveButton = new QPushButton;              //保存按钮
    clearButton = new QPushButton;             //清除按钮
    showHexCheckBox = new QCheckBox;           //显示16进制设置
    showSettingGroup = new QGroupBox;          //显示设置
    textBrowser = new QTextBrowser;            //文本浏览框
    sendSettingGroup = new QGroupBox;          //发送设置
    newLineBox = new QCheckBox;                //发送新行
    hexBox = new QCheckBox;                    //16进制发送
    cycleValueEdit = new QSpinBox;             //周期编辑框
    textEdit = new QTextEdit;                  //发送文本框
    sendButton = new QPushButton;              //发送按钮
    clearSendButton = new QPushButton;         //清除发送按钮
    stopSendButton = new QPushButton;          //停止发送按钮

    this->init();

    connect(saveButton, &QPushButton::clicked, this, [=](){
        QFileDialog save;
        save.setWindowTitle("保存");
        save.setDirectory(".");
        save.setDefaultSuffix("txt");
        save.setNameFilter("Text files (*.txt)");
        if(save.exec() == QDialog::Accepted)
        {
            QString path = save.selectedFiles()[0];
            QFile file(path);
            file.open(QIODevice::WriteOnly);
            file.write(textBrowser->toPlainText().toUtf8());
        }
    });

    connect(clearButton, &QPushButton::clicked, this, [=](){
        textBrowser->clear();
        data.clear();
    });

    connect(showHexCheckBox, &QCheckBox::toggled, this, [=](){
        showInHex();
        textBrowser->clear();
        textBrowser->setPlainText(data);
    });

    connect(serial, &Serial::readyRead, this, [=](){
        getSerialData();
        textBrowser->setPlainText(data);
    });

    connect(newLineBox, &QCheckBox::toggled, this, [=](){
        sendNewLine = !sendNewLine;
    });

    connect(hexBox, &QCheckBox::toggled, this, [=](){
        sendHex = !sendHex;
    });

    connect(sendButton, &QPushButton::clicked, this, [=](){
        auto data_ = textEdit->toPlainText();
        sendSerialData(data_);
    });

    connect(clearSendButton, &QPushButton::clicked, this, [=](){
        textEdit->clear();
    });

    timer = new QTimer;
    connect(timer, &QTimer::timeout, this, [=](){
        auto data_ = textEdit->toPlainText();
        if(!serial->getSerialPort()->isOpen())
            timer->stop();
        sendSerialData(data_);
    });

    connect(stopSendButton, &QPushButton::clicked, this, [=](){
        int cycle = cycleValueEdit->value();
        if(stopSendButton->text() == QString("停止发送"))
        {
            stopSendButton->setText("开始发送");
            timer->stop();
        }
        else if (stopSendButton->text() == QString("开始发送"))
        {
            timer->start(cycle);
            stopSendButton->setText("停止发送");
        }
    });
}

void DataWidget::showInHex()
{
    hexShowFlag = !hexShowFlag;
    if(hexShowFlag)
    {
        data = data.toUtf8().toHex(' ').toUpper();
        data.append(' ');
    }
    else
    {
        QStringList tmp = data.split(' ');
        data.clear();
        for(const QString & i : tmp)
        {
            char value = static_cast<char>(i.toInt(nullptr, 16));
            data.append(value);
        }
    }
}

void DataWidget::sendSerialData(QString & data_)
{
    QByteArray sendData;
    if(serial->getSerialPort()->isOpen())
    {
        if(sendNewLine)
        {
            data_.append("\r");
            data_.append("\n");
            sendData = QByteArray::fromHex(data_.toLatin1().toHex());
            serial->getSerialPort()->write(sendData);
            return;
        }
        if(sendHex)
        {
            sendData = QByteArray::fromHex(data_.toUtf8());
            serial->getSerialPort()->write(sendData);
            return;
        }
        sendData = QByteArray::fromHex(data_.toLatin1().toHex());
        serial->getSerialPort()->write(sendData);
    }
    else {
        QMessageBox::warning(this, "警告", "串口已关闭！");
        stopSendButton->setText("开始发送");
        timer->stop();
    }
}

void DataWidget::getSerialData()
{
    if (hexShowFlag)
        data.append(serial->read(true) + " ");
    else
        data.append(serial->read(false));
}

void DataWidget::init()
{
    auto * widget_1 = new QWidget(this);
    auto * widget_2 = new QWidget(this);
    auto * showSettingWidget = new QWidget(widget_1);
    auto * showButtonWidget = new QWidget(showSettingWidget);
    auto * sendButtonWidget = new QWidget(widget_2);

    QFont font(QStringLiteral("微软雅黑"), 10);
    //按钮样式
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

    //文本浏览框样式
    QString textBrowserStyle = QString::fromUtf8("QTextBrowser{\n"
                                                 "    border: 2px solid black;\n"
                                                 "    font-size:14px;\n"
                                                 "}");

    //文本编辑框样式
    QString textEditStyle = QString::fromUtf8("QTextEdit{\n"
                                              "    border: 2px solid black;\n"
                                              "    font-size:14px;\n"
                                              "}");

    //单行文本编辑框样式
    QString ValueEditStyle = QString::fromUtf8("QSpinBox{\n"
                                              "     border-radius:5px 5px 0px 0px;\n"
                                              "     border: 1px solid black;\n"
                                              "     font-size:14px;\n"
                                              "}");

    saveButton->setParent(showButtonWidget);
    saveButton->setFont(font);
    saveButton->setStyleSheet(buttonStyle);
    saveButton->setText("保存窗口");

    clearButton->setParent(showButtonWidget);
    clearButton->setFont(font);
    clearButton->setStyleSheet(buttonStyle);
    clearButton->setText("清除窗口");

    showSettingGroup->setParent(showSettingWidget);
    showSettingGroup->setFont(font);
    showSettingGroup->setTitle("显示设置");

    showHexCheckBox->setParent(showSettingGroup);
    showHexCheckBox->setFont(font);
    showHexCheckBox->setText("显示16进制");

    textBrowser->setParent(widget_1);
    textBrowser->setStyleSheet(textBrowserStyle);

    sendSettingGroup->setParent(widget_2);
    sendSettingGroup->setTitle("发送设置");

    newLineBox->setParent(sendSettingGroup);
    newLineBox->setFont(font);
    newLineBox->setText("发送新行");

    hexBox->setParent(sendSettingGroup);
    hexBox->setFont(font);
    hexBox->setText("发送16进制");

    auto * cycleLabel = new QLabel(sendSettingGroup);
    cycleLabel->setFont(font);
    cycleLabel->setText("周期:");

    cycleValueEdit->setParent(sendSettingGroup);
    cycleValueEdit->setFont(font);
    cycleValueEdit->setStyleSheet(ValueEditStyle);
    cycleValueEdit->setFixedHeight(40);
    cycleValueEdit->setFixedWidth(200);
    cycleValueEdit->setMinimum(1);
    cycleValueEdit->setMaximum(100000);
    cycleValueEdit->setSuffix("ms");

    textEdit->setParent(widget_2);
    textEdit->setFixedHeight(200);
    textEdit->setStyleSheet(textEditStyle);

    sendButton->setParent(sendButtonWidget);
    sendButton->setStyleSheet(buttonStyle);
    sendButton->setFont(font);
    sendButton->setText("发送");

    clearSendButton->setParent(sendButtonWidget);
    clearSendButton->setStyleSheet(buttonStyle);
    clearSendButton->setFont(font);
    clearSendButton->setText("清除发送");

    stopSendButton->setParent(sendButtonWidget);
    stopSendButton->setStyleSheet(buttonStyle);
    stopSendButton->setFont(font);
    stopSendButton->setText("开始发送");

    auto * showButtonWidgetLayout = new QHBoxLayout(showButtonWidget);
    showButtonWidgetLayout->addWidget(saveButton);
    showButtonWidgetLayout->addSpacing(20);
    showButtonWidgetLayout->addWidget(clearButton);

    //显示设置布局
    auto * showSettingLayout = new QFormLayout(showSettingGroup);
    showSettingLayout->setWidget(0, QFormLayout::LabelRole, showHexCheckBox);

    //上半窗口左侧布局
    auto * gridLayout = new QGridLayout(showSettingWidget);
    gridLayout->addWidget(settingGroup, 0, 0);
    gridLayout->addWidget(showButtonWidget, 1, 0);
    gridLayout->addWidget(showSettingGroup, 2, 0);


    //上半（显示）窗口布局
    auto * upLayout = new QHBoxLayout(widget_1);
    upLayout->addWidget(showSettingWidget);
    upLayout->addWidget(textBrowser);

    //发送设置窗口布局
    auto * sendSettingLayout = new QFormLayout(sendSettingGroup);
    sendSettingLayout->setWidget(0, QFormLayout::LabelRole, newLineBox);
    sendSettingLayout->setWidget(1, QFormLayout::LabelRole, hexBox);
    sendSettingLayout->setWidget(2, QFormLayout::LabelRole, cycleLabel);
    sendSettingLayout->setWidget(3, QFormLayout::LabelRole, cycleValueEdit);

    //发送按钮组布局
    auto * sendButtonLayout = new QVBoxLayout(sendButtonWidget);
    sendButtonLayout->addWidget(sendButton);
    sendButtonLayout->addWidget(clearSendButton);
    sendButtonLayout->addWidget(stopSendButton);

    //下半窗口布局
    auto * downLayout = new QGridLayout(widget_2);
    downLayout->addWidget(sendSettingGroup, 0, 0, 2, 2);
    downLayout->addWidget(textEdit, 0, 3, 2, 5);
    downLayout->addWidget(sendButtonWidget, 0, 8, 2, 1);

    //总窗口布局
    auto * lastLayout = new QVBoxLayout(this);
    lastLayout->addWidget(widget_1);
    lastLayout->addWidget(widget_2);
}

void DataWidget::paintEvent(QPaintEvent * e)
{
    QPainter painter(this);

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRect(this->rect());
}
