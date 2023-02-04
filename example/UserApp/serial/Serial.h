//
// Created by fazhehy on 2023/2/4.
//

#ifndef EXAMPLE_SERIAL_H
#define EXAMPLE_SERIAL_H

#include "usart.h"
#include <string>

class Serial {
public:
    class frame{
    private:
        uint8_t temp[4] = {0};
        uint8_t * frameData = nullptr;
        bool * boolData = nullptr;
        uint8_t * byteData = nullptr;
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
            delete [] frameData;
            Size = (boolDataNum + 7) / 8 + byteDataNum + shortDataNum * 2 + intDataNum * 4 + floatDataNum * 4 + 4;
            frameData = new uint8_t [Size];
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
    public:
        frame();
        explicit frame(int boolNum, int byteNum, int shortNum, int intNum, int floatNum);

        ~frame();

        [[nodiscard]] bool getCommandModeFlag() const
        {return commandMode;}
        void setCommandMode()
        {commandMode = true;}
        void resetCommandMode()
        {commandMode = false;}
        [[nodiscard]] char readCommand() const
        {return command;}
        void writeCommand(char c)
        {command = c;}
        uint8_t getVerifyCode();
        void resize(int boolNum ,int byteNum, int shortNum, int intNum, int floatNum);
        void clear();
        frame copy(const frame & f);
        void analysisFrame();
        [[nodiscard]] uint8_t * data() const
        {return frameData;}

        bool at(int position, bool *p);
        uint8_t at(int position, uint8_t *p);
        short at(int position, short *p);
        int at(int position, int *p);
        float at(int position, float *p);

        static uint8_t * float2byte(float * p);
        static uint8_t * int2byte(int * p);
        static uint8_t * short2byte(short * p);
        static float byte2float(uint8_t * arr);
        static int byte2int(uint8_t * arr);
        static short byte2short(uint8_t * arr);

        friend frame & operator<<(frame & f, bool n);
        friend frame & operator<<(frame & f, uint8_t n);
        friend frame & operator<<(frame & f, short n);
        friend frame & operator<<(frame & f, int n);
        friend frame & operator<<(frame & f, float n);

        frame & operator=(const frame & f);

        uint8_t operator[](int position);
    };
private:
    UART_HandleTypeDef * huart = nullptr;
    uint8_t * readBuffer = nullptr;
    uint8_t * writeBuffer = nullptr;
    uint8_t readBufferSize = 0;
    uint8_t writeBufferSize = 0;

    bool protocolFlag = false;
    enum State {ERROR=0, DOING=1, SUCCESS=2};
    State state = ERROR;
    frame readFrame;
    frame writeFrame;
public:
    explicit Serial();
    explicit Serial(UART_HandleTypeDef * pHuart);

    void init(UART_HandleTypeDef *pHuart);
    void interruptCallback(uint8_t data);

    void resizeReadBuffer(uint8_t size);
    void resizeWriteBuffer(uint8_t size);
    bool read(uint8_t * addr);
    bool read(uint8_t * addr, int size);
    bool write(uint8_t * addr);
    bool write(uint8_t * addr, int size);

    void setProtocol(const std::string & argumentList);
    void setProtocol();
    void resetProtocol()
    {protocolFlag = false;}
    frame & readFrameData();
    frame & writeFrameData();
    void sendFrame();

    ~Serial();
};

#endif //EXAMPLE_SERIAL_H
