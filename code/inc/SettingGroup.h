//
// Created by fazhehy on 2023/1/26.
//

#ifndef SERIALASSISTANT_SETTINGGROUP_H
#define SERIALASSISTANT_SETTINGGROUP_H

#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include "Serial.h"

class SettingGroup : public QGroupBox{
    Q_OBJECT
private:
    Serial * serial;
    QComboBox * serialNameComBox;
    QComboBox * baudComBox;
    QComboBox * stopBitsComBox;
    QComboBox * dataBitsComBox;
    QComboBox * parityComBox;
    QPushButton * serialStateButton;

    QStringList serialNameList;
    QStringList newSerialName;
    QStringList removeSerialName;
public:
    SettingGroup(Serial * pSerial, QWidget * parent);
    void init();
    void updateSerialNameList();
    void reset();
};


#endif //SERIALASSISTANT_SETTINGGROUP_H
