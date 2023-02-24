//
// Created by fazhehy on 2023/1/25.
//

#include <QMap>
#include <QSerialPortInfo>
#include <QDebug>
#include "Serial.h"

char Serial::frame::getVerifyCode()
{
    int sum = 0;
    for (char & i : frameData) {
        sum += i;
    }
    return static_cast<char>(sum&0xff);
}

void Serial::frame::setFrame()
{
    frameData.clear();
    frameData.append(0x5a);
    if(!commandMode)
    {
        frameData.append('\0');
        if (boolDataNum != 0)
        {
            if (boolDataNum <= 8)
            {
                char t = 0;
                for (int i = 0; i < boolDataNum; ++i)
                {
                    t |= boolData[i];
                    t <<= 1;
                }
                frameData.append(t);
            }
            else
            {
                int n = 0;
                while(boolDataNum - 8*(n+1) > 0)
                {
                    char t = 0;
                    for (int i = 0; i < 8; ++i)
                    {
                        t |= boolData[i];
                        t <<= 1;
                    }
                    frameData.append(t);
                    n++;
                }
                char t = 0;
                for (int i = 0; i < boolDataNum - 8*n; ++i)
                {
                    t |= boolData[i];
                    t <<= 1;
                }
                frameData.append(t);
            }
        }

        for (int i = 0; i < byteDataNum; ++i) {
            frameData.append(byteData[i]);
        }

        for (int i = 0; i < shortDataNum; ++i) {
            frameData.append(short2byte(shortData+i));
        }

        for (int i = 0; i < intDataNum; ++i) {
            frameData.append(int2byte(intData+i));
        }

        for (int i = 0; i < floatDataNum; i+=1) {
            frameData.append(float2byte(floatData+i));
        }

        frameData.append(getVerifyCode());
        frameData.append(0xa5);
    }
    else
    {
        frameData.append(0x01);
        frameData.append(command);
        frameData.append(0xa5);
    }
}

void Serial::frame::analysisFrame()
{
    currentBoolNum = 0;
    currentByteNum = 0;
    currentShortNum = 0;
    currentIntNum = 0;
    currentFloatNum = 0;

    QByteArray tmp = frameData;
    if (frameData[1] == 0x01)
        commandMode = true;
    if(!commandMode)
    {
        tmp.remove(tmp.size()-2, 2);

        for (int i = floatDataNum-1; i >= 0; --i) {
            floatData[i] = byte2float(tmp.right(4).data());
            tmp.remove(tmp.size()-4, 4);
        }

        for (int i = intDataNum-1; i >= 0 ; --i) {
            intData[i] = byte2int(tmp.right(4).data());
            tmp.remove(tmp.size()-4, 4);
        }

        for (int i = shortDataNum-1; i >= 0 ; --i) {
            shortData[i] = byte2short(tmp.right(2).data());
            tmp.remove(tmp.size()-2, 2);
        }

        for (int i = byteDataNum-1; i >= 0 ; --i) {
            byteData[i] = *tmp.right(1).data();
            tmp.remove(tmp.size()-1, 1);
        }

        if(boolDataNum != 0) {
            if (boolDataNum <= 8) {
                char t = *tmp.right(1).data();
                for (int i = boolDataNum - 1; i >= 0; --i) {
                    boolData[i] = (bool) (t >> i) & 0xfe;
                }
                tmp.remove(tmp.size() - 1, 1);
            } else {
                int n = 1;
                while (boolDataNum - 8 * n > 0)
                    n++;

                auto tmp1 = tmp.right(n);
                long long a = tmp1.toHex().toLongLong(nullptr, 16);
                n = boolDataNum;
                while (n != 0) {
                    boolData[n] = (a >> n) & 0xfe;
                    n--;
                }
            }
        }
    }
    else
    {
        command = tmp[2];
        tmp.clear();
    }
    frameData.clear();
}

Serial::frame::frame(int boolNum, int byteNum, int shortNum, int intNum, int floatNum) : boolDataNum(boolNum), byteDataNum(byteNum), shortDataNum(shortNum), intDataNum(intNum), floatDataNum(floatNum)
{
    boolData = new bool [boolNum];
    byteData = new char [byteNum];
    shortData = new short [shortNum];
    intData = new int [intNum];
    floatData = new float [floatNum];
}

Serial::frame::frame()
{
    boolData = new bool [1];
    byteData = new char [1];
    shortData = new short [1];
    intData = new int [1];
    floatData = new float [1];
}

Serial::frame::~frame()
{
    delete [] boolData;
    delete [] byteData;
    delete [] shortData;
    delete [] intData;
    delete [] floatData;
}

void Serial::frame::clear()
{
    currentBoolNum = 0;
    currentByteNum = 0;
    currentShortNum = 0;
    currentIntNum = 0;
    currentFloatNum = 0;
    frameData.clear();
}

void Serial::frame::resize(int boolNum, int byteNum, int shortNum, int intNum, int floatNum)
{
    delete [] boolData;
    boolData = nullptr;
    delete [] byteData;
    byteData = nullptr;
    delete [] shortData;
    shortData = nullptr;
    delete [] intData;
    intData = nullptr;
    delete [] floatData;
    floatData = nullptr;

    boolDataNum = boolNum;
    byteDataNum = byteNum;
    shortDataNum = shortNum;
    intDataNum = intNum;
    floatDataNum = floatNum;

    boolData = new bool [boolNum];
    byteData = new char [byteNum];
    shortData = new short [shortNum];
    intData = new int [intNum];
    floatData = new float [floatNum];

    clear();
    setSize();
}

Serial::frame Serial::frame::copy(const Serial::frame &f)
{
    this->resize(f.BoolNum(), f.ByteNum(), f.ShortNum(), f.IntNum(), f.FloatNum());
    memcpy(f.BoolData(), this->BoolData(), (f.BoolNum()+8)/8);
    memcpy(f.ByteData(), this->ByteData(), f.ByteNum());
    memcpy(f.ShortData(), this->ShortData(), f.ShortNum()*2);
    memcpy(f.IntData(), this->IntData(), f.IntNum()*4);
    memcpy(f.FloatData(), this->FloatData(), f.FloatNum()*4);
    this->frameData = f.frameData;

    return *this;
}

bool Serial::frame::at(int position, bool *p)
{
    if(position > boolDataNum)
    {
        *p = false;
        return false;
    }
    else
    {
        *p = boolData[position-1];
        return boolData[position-1];
    }
}

char Serial::frame::at(int position, char *p)
{
    if(position > byteDataNum)
    {
        *p = 0;
        return 0;
    }
    else
    {
        *p = byteData[position-1];
        return byteData[position-1];
    }
}

short Serial::frame::at(int position, short *p)
{
    if(position > shortDataNum)
    {
        *p = 0;
        return 0;
    }
    else
    {
        *p = shortData[position-1];
        return shortData[position-1];
    }
}

int Serial::frame::at(int position, int *p)
{
    if(position > intDataNum)
    {
        *p = 0;
        return 0;
    }
    else
    {
        *p = intData[position-1];
        return intData[position-1];
    }
}

float Serial::frame::at(int position, float *p)
{
    if(position > floatDataNum)
    {
        *p = 0;
        return 0;
    }
    else
    {
        *p = floatData[position-1];
        return floatData[position-1];
    }
}

Serial::frame & operator<<(Serial::frame & f, bool n)
{
    f.currentBoolNum++;
    if(f.currentBoolNum > f.boolDataNum || f.boolData == nullptr)
        return f;
    f.boolData[f.currentBoolNum-1] = n;
    f.setFrame();
    return f;
}

Serial::frame & operator<<(Serial::frame & f, char n)
{
    if (!f.getCommandModeFlag())
    {
        f.currentByteNum++;
        if(f.currentByteNum > f.byteDataNum || f.byteData == nullptr)
            return f;
        f.byteData[f.currentByteNum-1] = n;
        f.setFrame();
        return f;
    }
    else
    {
        f.command = n;
        f.setFrame();
        return f;
    }
}

Serial::frame & operator<<(Serial::frame & f, short n)
{
    f.currentShortNum++;
    if(f.currentShortNum > f.shortDataNum || f.shortData == nullptr)
        return f;
    f.shortData[f.currentShortNum-1] = n;
    f.setFrame();
    return f;
}

Serial::frame & operator<<(Serial::frame & f, int n)
{
    f.currentIntNum++;
    if(f.currentIntNum > f.intDataNum || f.intData == nullptr)
        return f;
    f.intData[f.currentIntNum-1] = n;
    f.setFrame();
    return f;
}

Serial::frame & operator<<(Serial::frame & f, float n)
{
    f.currentFloatNum++;
    if(f.currentFloatNum > f.floatDataNum || f.floatData == nullptr)
        return f;
    f.floatData[f.currentFloatNum-1] = n;
    f.setFrame();
    return f;
}

Serial::frame & Serial::frame::operator=(const Serial::frame &f)
{
    this->copy(f);
    return *this;
}

char Serial::frame::operator[](int position)
{
    return frameData.at(position);
}

QByteArray Serial::frame::int2byte(int *p)
{
    return {reinterpret_cast<char *>(p), 4};
}

QByteArray Serial::frame::float2byte(float *p)
{
    return {reinterpret_cast<char *>(p), 4};
}

QByteArray Serial::frame::short2byte(short *p)
{
    return {reinterpret_cast<char *>(p), 2};
}

int Serial::frame::byte2int(char * arr)
{
    auto i = reinterpret_cast<int *>(arr);
    return *i;
}

float Serial::frame::byte2float(char *arr)
{
    auto f = reinterpret_cast<float *>(arr);
    return *f;
}

short Serial::frame::byte2short(char *arr)
{
    auto i = reinterpret_cast<short *>(arr);
    return *i;
}

Serial::Serial()
{
    serial = new QSerialPort;
    protocolFlag = false;
    connect(serial, &QSerialPort::readyRead, this, [=](){readSlot();});
}

void Serial::init(const QString& name,
                  QSerialPort::BaudRate baudRate,
                  QSerialPort::StopBits stopBits,
                  QSerialPort::DataBits dataBits,
                  QSerialPort::Parity parity)
{
    serial->setPortName(name);
    serial->setBaudRate(baudRate);
    serial->setStopBits(stopBits);
    serial->setParity(parity);
}

void Serial::readSlot()
{
    static int n = 0;
    if(protocolFlag)
    {
        int size = readFrame.size();
        char c = static_cast<char>(serial->read(1).toHex().toInt(nullptr, 16));
        if(state == ERROR)
        {
            readFrame.clear();
            n = 0;
        }
        if(!readFrame.getCommandModeFlag())
        {
            if (n == 0){
                if (c == 0x5a)
                    state = DOING;
                else
                    state = ERROR;
            }
            else if(n == size-2)
            {
                if (c == readFrame.getVerifyCode())
                    state = DOING;
                else
                    state = ERROR;
            }
            else if(n == size-1)
            {
                if (uint8_t(c) == 0xa5)
                    state = SUCCESS;
                else
                    state = ERROR;
            }
            n++;
            readFrame.data().append(c);
        }
        else
        {
            if(n == 0)
            {
                if (c == 0x5a)
                  state = DOING;
                else
                  state = ERROR;
            }
            else if(n == 3)
            {
                if (uint8_t(c) == 0xa5)
                  state = SUCCESS;
                else
                  state = ERROR;
            }
            n++;
            readFrame.data().append(c);
        }
        if(state == SUCCESS)
        {
//            qDebug() << readFrame.data().toHex(' ');
            readFrame.analysisFrame();
            state = ERROR;
            emit analysisOver();
        }
    }
    else
        emit readyRead();

}

void Serial::setReadFrameSize(int size)
{
    serial->setReadBufferSize(size);
    readBuffer.resize(size, 0);
}

//void Serial::setWriteFrameSize(int size)
//{
//    writeBuffer.resize(size, 0);
//}

QByteArray Serial::read()
{
//    return readBuffer;
    return serial->readAll();
}

QString Serial::read(bool showHex)
{
    QString tmp;
    if (!showHex){
       tmp = serial->readAll();
        return tmp;
    } else
        return serial->readAll().toHex(' ').toUpper();
}

void Serial::reset()
{
    readBuffer.clear();
//    writeBuffer.clear();
    readFrame.clear();
    writeFrame.clear();
    serial->close();
    serial->setReadBufferSize(0);
}

QStringList Serial::getAvailableSerialName()
{
    QStringList serialName;
    for(auto & info : QSerialPortInfo::availablePorts())
        serialName += info.portName();

    return serialName;
}

void Serial::setProtocol(const QString& argumentList)
{
    int rBoolNum = 0, rByteNum = 0, rShortNum = 0, rIntNum = 0, rFloatNum=0;
    int wBoolNum = 0, wByteNum = 0, wShortNum = 0, wIntNum = 0, wFloatNum=0;
    QMap<int, QString> arg_map;
    int position = (int)argumentList.indexOf("read", 0, Qt::CaseInsensitive);
    if (position != -1)
    {
        position += 5;
        int size = (int)argumentList.indexOf("}", position, Qt::CaseInsensitive)-position;
        arg_map.insert(0, argumentList.mid(position, size));
    }
    position = (int)argumentList.indexOf("write", 0, Qt::CaseInsensitive);
    if (position != -1)
    {
        position += 6;
        int size = (int)argumentList.indexOf("}", position, Qt::CaseInsensitive)-position;
        arg_map.insert(1, argumentList.mid(position, size));
    }

    for (auto item = arg_map.begin(); item != arg_map.end(); ++item) {
        QMap<QString, int> map;
        for (auto & s: item.value().split(";", Qt::SkipEmptyParts))
        {
            auto list = s.split(':');
            map.insert(list[0].remove(' '), list[1].remove(' ').toInt(nullptr, 10));
        }
        for (auto i = map.begin(); i != map.end(); ++i)
        {
            if (i.key() == "bool")
            {
                if(item.key() == 0)
                    rBoolNum = i.value();
                else
                    wBoolNum = i.value();
            }
            else if(i.key() == "byte")
            {
                if(item.key() == 0)
                    rByteNum = i.value();
                else
                    wByteNum = i.value();
            }
            else if(i.key() == "short")
            {
                if (item.key() == 0)
                    rShortNum = i.value();
                else
                    wShortNum = i.value();
            }
            else if(i.key() == "int" )
            {
                if(item.key() == 0)
                    rIntNum = i.value();
                else
                    wIntNum = i.value();
            }
            else if(i.key() == "float")
            {
                if(item.key() == 0)
                    rFloatNum = i.value();
                else
                    wFloatNum = i.value();
            }
        }
    }
    readFrame.resize(rBoolNum, rByteNum, rShortNum, rIntNum, rFloatNum);
    writeFrame.resize(wBoolNum, wByteNum, wShortNum, wIntNum, wFloatNum);
    protocolFlag = true;
    serial->setReadBufferSize(1);
}

void Serial::setProtocol()
{
//    bool:1; byte:1; short:1; int:i; float:1;
    readFrame.resize(1, 1, 1, 1, 1);
    writeFrame.resize(0, 0, 0, 0, 0);

    protocolFlag = true;
    serial->setReadBufferSize(1);
}

Serial::frame & Serial::readFrameData()
{
    return readFrame;
}

Serial::frame & Serial::writeFrameData()
{
    return writeFrame;
}

void Serial::sendFrame()
{
    qDebug() << writeFrame.data().toHex(' ');
    serial->write(writeFrame.data());
    writeFrame.clear();
}

Serial::~Serial()
{
    delete serial;
}
