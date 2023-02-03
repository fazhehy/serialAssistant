//
// Created by fazhehy on 2023/1/25.
//

#include "MainWindow.h"
#include <QHBoxLayout>
#include <QFont>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    serial = new Serial;
    tabWidget = new QTabWidget;
    dataWidget = new DataWidget(serial);
    pidWidget = new PIDWidget(serial);

    this->init();

    void (QTabWidget:: * tabSignal)(int) = &QTabWidget::currentChanged;
    connect(tabWidget, tabSignal, this, [=](){
        serial->reset();
        if (tabWidget->currentIndex() == 0)
            serial->resetProtocol();
        else if(tabWidget->currentIndex() == 1)
            serial->setProtocol("read{"
                                    "float:2;"
                                    "}"
                                "write{"
                                    "float:4;"
                                    "}");
    });
}

void MainWindow::init()
{
    this->resize(800, 600);

    auto * centralWidget = new QWidget;
    tabWidget->setParent(centralWidget);

    tabWidget->setFont(QFont(QStringLiteral("微软雅黑"), 12));
    tabWidget->setTabShape(QTabWidget::Triangular);

    tabWidget->addTab(dataWidget, "数据收发");
    tabWidget->addTab(pidWidget, "PID调参");

    auto * layout = new QHBoxLayout(centralWidget);
    layout->addWidget(tabWidget);

    this->setCentralWidget(centralWidget);
}
