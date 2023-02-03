//
// Created by fazhehy on 2023/1/25.
//

#ifndef SERIALASSISTANT_MAINWINDOW_H
#define SERIALASSISTANT_MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include "Serial.h"
#include "DataWidget.h"
#include "PIDWidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    Serial * serial;
    QTabWidget * tabWidget;
    DataWidget * dataWidget;
    PIDWidget * pidWidget;
public:
    explicit MainWindow(QWidget * parent = nullptr);
    void init();
};


#endif //SERIALASSISTANT_MAINWINDOW_H
