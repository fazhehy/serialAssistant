//
// Created by fazhehy on 2023/1/25.
//

#ifndef SERIALASSISTANT_SERIAL_H
#define SERIALASSISTANT_SERIAL_H

#include <QSerialPort>

class Serial : public QObject{
    Q_OBJECT
public:
    class frame{
    private:
        QByteArray frameData;
        bool * boolData = nullptr;
        char * byteData = nullptr;
        short * shortData = nullptr;
        int * intData = nullptr;
        float * floatData = nullptr;
        int boolDataNum = 0;
        int byteDataNum = 0;
        int shortDataNum = 0;
        int intDataNum = 0;
        int floatDataNum = 0;
        int currentBoolNum = 0;
        int currentByteNum = 0;
        int currentShortNum = 0;
        int currentIntNum = 0;
        int currentFloatNum = 0;
        int Size = 0;
        bool commandMode = false;
        char command;
    private:
        void setSize() {
            Size = (boolDataNum + 7) / 8 + byteDataNum + shortDataNum * 2 + intDataNum * 4 + floatDataNum * 4 + 4;
//            frameData.resize((boolDataNum+8)/8 + byteDataNum + shortDataNum*2 + intDataNum*4 + floatDataNum*4);}
        }
        void setFrame();
    public:
        [[nodiscard]] int size() const
        {return Size;}
        [[nodiscard]] int BoolNum() const
        {return boolDataNum;}
        [[nodiscard]] int ByteNum() const
        {return byteDataNum;}
        [[nodiscard]] int ShortNum() const
        {return shortDataNum;}
        [[nodiscard]] int IntNum() const
        {return intDataNum;}
        [[nodiscard]] int FloatNum() const
        {return floatDataNum;}
        [[nodiscard]] bool * BoolData() const
        {return boolData;}
        [[nodiscard]] char * ByteData() const
        {return byteData;}
        [[nodiscard]] short * ShortData() const
        {return shortData;}
        [[nodiscard]] int * IntData() const
        {return intData;}
        [[nodiscard]] float * FloatData() const
        {return floatData;}
    public:
        frame();
        explicit frame(int boolNum,
              int byteNum,
              int shortNum,
              int intNum,
              int floatNum);

        ~frame();

        bool getCommandModeFlag()
        {return commandMode;}
        void setCommandMode()
        {commandMode = true;}
        void resetCommandMode()
        {commandMode = false;}
        char readCommand()
        {return command;}
        void writeCommand(char c)
        {command = c;}
        char getVerifyCode();
        void resize(int boolNum ,int byteNum, int shortNum, int intNum, int floatNum);
        void clear();
        frame copy(const frame & f);
        void analysisFrame();
        QByteArray& data()
        {return frameData;}

        bool at(int position, bool *p);
        char at(int position, char *p);
        short at(int position, short *p);
        int at(int position, int *p);
        float at(int position, float *p);

        static QByteArray float2byte(float * p);
        static QByteArray int2byte(int * p);
        static QByteArray short2byte(short * p);
        static float byte2float(char * arr);
        static int byte2int(char * arr);
        static short byte2short(char * arr);

        friend frame & operator<<(frame & f, bool n);
        friend frame & operator<<(frame & f, char n);
        friend frame & operator<<(frame & f, short n);
        friend frame & operator<<(frame & f, int n);
        friend frame & operator<<(frame & f, float n);

        frame & operator=(const frame & f);

        char operator[](int position);
    };
private:
    QSerialPort * serial = nullptr;

    QByteArray readBuffer;
//    QByteArray writeBuffer;

    bool protocolFlag = false;
    enum State {ERROR=0, DOING=1, SUCCESS=2};
    State state = ERROR;
    frame readFrame;
    frame writeFrame;
public:
    explicit Serial();
    void init(const QString& name,
              QSerialPort::BaudRate baudRate = QSerialPort::Baud115200,
              QSerialPort::StopBits stopBits = QSerialPort::OneStop,
              QSerialPort::DataBits dataBits = QSerialPort::Data8,
              QSerialPort::Parity parity = QSerialPort::NoParity);
    void setReadFrameSize(int size = 1);
//    void setWriteFrameSize(int size = 1);
    QByteArray read();
    QString read(bool showHex);
    void reset();
    QSerialPort * getSerialPort()
    {return serial;}

    static QStringList getAvailableSerialName();

    void setProtocol(const QString& argumentList);
    void setProtocol();
    void resetProtocol()
    {protocolFlag = false;}
    frame & readFrameData();
    frame & writeFrameData();
    void sendFrame();

    ~Serial();
public slots:
    void readSlot();
signals:
    void readyRead();
    void analysisOver();
};

#endif //SERIALASSISTANT_SERIAL_H
