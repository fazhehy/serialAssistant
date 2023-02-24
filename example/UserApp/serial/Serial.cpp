//
// Created by fazhehy on 2023/2/4.
//

#include "Serial.h"
#include <cstring>
#include <map>
#include <vector>

uint8_t Serial::frame::getVerifyCode()
{
    uint32_t sum = 0;
    for (int i = 0; i < Size-2; ++i)
        sum += frameData[i];
    Sum = sum;
    return static_cast<uint8_t>(sum&0xff);
}

void Serial::frame::setFrame()
{
    int index = 1;
    frameData[0] = 0x5a;
    if(!commandMode)
    {
        frameData[1] = 0x00;
        index ++;
        if (boolDataNum != 0)
        {
            if (boolDataNum <= 8)
            {
                uint8_t t = 0;
                for (int i = 0; i < boolDataNum; ++i)
                {
                    t |= boolData[i];
                    t <<= 1;
                }
                frameData[index]=t;
                index ++;
            }
            else
            {
                for (int n = 0; n < (boolDataNum+7)/8-1; ++n) {
                    uint8_t t = 0;
                    for (int i = 0; i < 8; ++i)
                    {
                        t |= boolData[i];
                        t <<= 1;
                    }
                    frameData[index++] = t;
                }
                if (boolDataNum%8 != 0)
                {
                    uint8_t t = 0;
                    for (int i = (boolDataNum+7)/8-1; i < boolDataNum; ++i)
                    {
                        t |= boolData[i];
                        t <<= 1;
                    }
                    frameData[index++] = t;
                }
            }
        }

        for (int i = 0; i < byteDataNum; ++i)
            frameData[index+i] = byteData[i];
        index += byteDataNum;

        for (int i = 0; i < shortDataNum; ++i)
            memcpy(frameData+index+i*2, short2byte(shortData+i), 2);
        index += shortDataNum*2;

        for (int i = 0; i < intDataNum; ++i)
            memcpy(frameData+index+i*4, int2byte(intData+i), 4);
        index += intDataNum*4;

        for (int i = 0; i < floatDataNum; i+=1)
            memcpy(frameData+index+i*4, float2byte(floatData+i), 4);
        index += floatDataNum*4;

        frameData[index++] = getVerifyCode();
        frameData[index] = 0xa5;
    }
    else
    {
        frameData[1] = 0x01;
        frameData[2] = command;
        frameData[4] = 0x5a;
    }
}

void Serial::frame::analysisFrame()
{
    currentBoolNum = 0;
    currentByteNum = 0;
    currentShortNum = 0;
    currentIntNum = 0;
    currentFloatNum = 0;
    int index = 2;
    if (!commandMode)
    {
        if(boolDataNum != 0) {
            if (boolDataNum <= 8) {
                uint8_t t = frameData[2];
                for (int i = 0; i < boolDataNum; ++i) {
                    boolData[i] = (bool) (t >> i) & 0xfe;
                }
                index ++;
            } else {
                for(int n = (boolDataNum+7)/8; n > 0; --n)
                {
                    for (int i = 0; i < 8; ++i) {
                        boolData[8*n+i] = (frameData[n]>>i)&0xfe;
                    }
                }
                index += (boolDataNum+7)/8;
            }
        }

        for (int i = 0; i < byteDataNum ; ++i)
            byteData[i] = frameData[index+i];
        index += byteDataNum;

        for (int i = 0; i < shortDataNum ; ++i)
            shortData[i] = byte2short(frameData+index+i*2);
        index += shortDataNum*2;

        for (int i = 0; i < intDataNum ; ++i)
            intData[i] = byte2int(frameData+index+i*4);
        index += intDataNum*4;

        for (int i = floatDataNum-1; i >= 0; --i)
            floatData[i] = byte2float(frameData+index+i*4);
        index += floatDataNum*4;
    } else
        command = frameData[2];

    memset(frameData, 0, Size);
}


Serial::frame::frame(int boolNum, int byteNum, int shortNum, int intNum, int floatNum) : boolDataNum(boolNum), byteDataNum(byteNum), shortDataNum(shortNum), intDataNum(intNum), floatDataNum(floatNum)
{
    boolData = new bool [boolNum];
    byteData = new uint8_t [byteNum];
    shortData = new short [shortNum];
    intData = new int [intNum];
    floatData = new float [floatNum];
}

Serial::frame::frame()
{
    boolData = new bool [1];
    byteData = new uint8_t [1];
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
    memset(frameData, 0, Size);
}

void Serial::frame::resize(int boolNum, int byteNum, int shortNum, int intNum, int floatNum)
{
    delete [] boolData;
    delete [] byteData;
    delete [] shortData;
    delete [] intData;
    delete [] floatData;

    boolDataNum = boolNum;
    byteDataNum = byteNum;
    shortDataNum = shortNum;
    intDataNum = intNum;
    floatDataNum = floatNum;

    boolData = new bool [boolNum];
    byteData = new uint8_t [byteNum];
    shortData = new short [shortNum];
    intData = new int [intNum];
    floatData = new float [floatNum];

    clear();
    setSize();
}

Serial::frame Serial::frame::copy(const Serial::frame &f)
{
    this->resize(f.BoolNum(), f.ByteNum(), f.ShortNum(), f.IntNum(), f.FloatNum());
    memcpy(f.boolData, this->boolData, (f.BoolNum()+8)/8);
    memcpy(f.byteData, this->byteData, f.ByteNum());
    memcpy(f.shortData, this->shortData, f.ShortNum()*2);
    memcpy(f.intData, this->intData, f.IntNum()*4);
    memcpy(f.floatData, this->floatData, f.FloatNum()*4);
    memcpy(f.frameData, this->frameData, Size);

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

uint8_t Serial::frame::at(int position, uint8_t *p)
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

int Serial::frame::at(int position, uint32_t *p)
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

Serial::frame & operator<<(Serial::frame & f, uint8_t n)
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

uint8_t Serial::frame::operator[](int position)
{
    if (position >= Size)
        return 0;
    return frameData[position];
}

uint8_t * Serial::frame::float2byte(float *p)
{
    return reinterpret_cast<uint8_t *>(p);
}

uint8_t * Serial::frame::int2byte(int *p)
{
    return reinterpret_cast<uint8_t *>(p);
}

uint8_t * Serial::frame::short2byte(short *p)
{
    return reinterpret_cast<uint8_t *>(p);
}

int Serial::frame::byte2int(uint8_t * arr)
{
    auto i = reinterpret_cast<int *>(arr);
    return *i;
}

float Serial::frame::byte2float(uint8_t *arr)
{
    auto f = reinterpret_cast<float *>(arr);
    return *f;
}

short Serial::frame::byte2short(uint8_t *arr)
{
    auto i = reinterpret_cast<short *>(arr);
    return *i;
}

Serial::Serial()
{
    readBuffer = new uint8_t [1];
    readBufferSize = 1;
    writeBuffer = new uint8_t [1];
    writeBufferSize = 1;
}

Serial::Serial(UART_HandleTypeDef *pHuart) : huart(pHuart)
{
    readBuffer = new uint8_t [1];
    readBufferSize = 1;
    writeBuffer = new uint8_t [1];
    writeBufferSize = 1;
}

void Serial::init(UART_HandleTypeDef *pHuart)
{
    huart = pHuart;
}

void Serial::interruptCallback(uint8_t data)
{
    static int n = 0;
    static int index = 0;
    static bool flag = false;
    int size = readFrame.size();
    readFrame.resetCommandMode();
    if(protocolFlag)
    {
        if(state == ERROR)
        {
            readFrame.clear();
            n = 0;
        }
        if (n == 0){
            if (data == 0x5a)
                state = DOING;
            else
                state = ERROR;
        }
        else if(n == 1)
        {
            if (data == 0x00)
                flag = false;
            else if (data == 0x01)
                flag = true;
        }
        if (!flag)
        {
            if(n == size-2)
            {
                if (data == readFrame.getVerifyCode())
                    state = DOING;
                else
                    state = ERROR;
            }
            else if(n == size-1)
            {
                if (data == 0xa5)
                    state = SUCCESS;
                else
                    state = ERROR;
            }
        }
        else
        {
            if(n == 3)
            {
                if (data == 0xa5)
                    state = SUCCESS;
                else
                    state = ERROR;
            }

        }
        readFrame.data()[n] = data;
        n++;
        if (state == SUCCESS)
        {
            if (flag)
                readFrame.setCommandMode();
            readFrame.analysisFrame();
            analysisFlag = SUCCESS;
            state = ERROR;
            flag = false;
        }
    }
    else
    {
        if(readBuffer != nullptr)
        {
            readBuffer[index++] = data;
            if (index > readBufferSize)
                index = 0;
        }
    }
}

void Serial::resizeReadBuffer(uint8_t size)
{
    delete [] readBuffer;
    readBuffer = new uint8_t [size];
    readBufferSize = size;
}

void Serial::resizeWriteBuffer(uint8_t size)
{
    delete [] writeBuffer;
    writeBuffer = new uint8_t [size];
    writeBufferSize = size;
}

bool Serial::read(uint8_t *addr)
{
    if (addr == nullptr)
        return false;
    else
        memcpy(addr, readBuffer, readBufferSize);
    return true;
}

bool Serial::read(uint8_t *addr, int size)
{
    resizeReadBuffer(size);
    return read(addr);
}

bool Serial::write(uint8_t *addr)
{
    if (addr == nullptr)
        return false;
    else
        memcpy(addr, writeBuffer, writeBufferSize);
    return true;
}


bool Serial::write(uint8_t *addr, int size)
{
    resizeWriteBuffer(size);
    return write(addr);
}

std::vector<std::string> split(std::string str, char ch) {
    int pos;
    std::vector<std::string> vec;
    do {
        pos = static_cast<int>(str.find(ch,0));
        if (pos != std::string::npos){
            vec.push_back(str.substr(0, pos));
            str.erase(0, pos+1);
        }
        else {
            if (!str.empty())
                vec.push_back(str.substr(0, str.size()));
        }
    }while(pos != std::string::npos);
    return vec;
}

std::string remove(std::string & str, char ch)
{
    int pos = 0;
    std::string tmp;
    do {
        pos = static_cast<int>(str.find(ch, 0));
        if (pos != std::string::npos)
        {
            tmp = str.substr(pos+1, str.size()-pos-1);
            str.erase(pos);
            str += tmp;
        }
    } while (pos != std::string::npos);
    return str;
}

void Serial::setProtocol(const std::string& argumentList)
{
    int rBoolNum = 0, rByteNum = 0, rShortNum = 0, rIntNum = 0, rFloatNum=0;
    int wBoolNum = 0, wByteNum = 0, wShortNum = 0, wIntNum = 0, wFloatNum=0;
    std::map<int, std::string> arg_map;
    int position = (int)argumentList.find("read", 0);
    if (position != std::string::npos)
    {
        position += 5;
        int size = (int)argumentList.find('}', position)-position;
        arg_map[0] = argumentList.substr(position, size);
    }
    position = (int)argumentList.find("write", 0);
    if (position != std::string::npos)
    {
        position += 6;
        int size = (int)argumentList.find('}', position)-position;
        arg_map[1] = argumentList.substr(position, size);
    }

    for (auto & item : arg_map) {
        std::map<std::string, int> map;
        for (auto & s: split(item.second, ';'))
        {
            auto list = split(s, ':');
            map[remove(list[0], ' ')] = atoi(list[1].c_str());
        }
        for (auto & i : map)
        {
            if (i.first == "bool")
            {
                if(item.first == 0)
                    rBoolNum = i.second;
                else
                    wBoolNum = i.second;
            }
            else if(i.first == "byte")
            {
                if(item.first == 0)
                    rByteNum = i.second;
                else
                    wByteNum = i.second;
            }
            else if(i.first == "short")
            {
                if (item.first == 0)
                    rShortNum = i.second;
                else
                    wShortNum = i.second;
            }
            else if(i.first == "int" )
            {
                if(item.first == 0)
                    rIntNum = i.second;
                else
                    wIntNum = i.second;
            }
            else if(i.first == "float")
            {
                if(item.first == 0)
                    rFloatNum = i.second;
                else
                    wFloatNum = i.second;
            }
        }
    }
    readFrame.resize(rBoolNum, rByteNum, rShortNum, rIntNum, rFloatNum);
    writeFrame.resize(wBoolNum, wByteNum, wShortNum, wIntNum, wFloatNum);
    protocolFlag = true;
}

void Serial::setProtocol()
{
    //    bool:1; byte:1; short:1; int:i; float:1;
    readFrame.resize(1, 1, 1, 1, 1);
    writeFrame.resize(0, 0, 0, 0, 0);

    protocolFlag = true;
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
    if (!writeFrame.getCommandModeFlag())
        HAL_UART_Transmit(huart, writeFrame.data(), writeFrame.size(), 1000);
    else
        HAL_UART_Transmit(huart, writeFrame.data(), 4, 1000);
    writeFrame.clear();
}

Serial::~Serial()
{
    delete [] readBuffer;
    delete [] writeBuffer;
}
