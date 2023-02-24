//
// Created by fazhehy on 2023/2/4.
//

#include "common_inc.h"

Serial serial(&huart1);
uint8_t data;
float p, i, d, target, value;
uint8_t command;
uint32_t time;

void Main()
{
    OLED_Init();
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_UART_Receive_IT(&huart1, &data, 1);
    uint8_t l = 0;
    serial.setProtocol("read{"
                       "    float:4;"
                       "    }"
                       "write{"
                       "    float:2;"
                       "    }");
    for(;;)
    {
        if (serial.analysisIsOver())
        {
            serial.readFrameData().at(1, &target);
            serial.readFrameData().at(2, &p);
            serial.readFrameData().at(3, &i);
            serial.readFrameData().at(4, &d);
            command = serial.readFrameData().readCommand();
            OLED_ShowFloatNum(1, 1, p);
            OLED_ShowFloatNum(2, 1, i);
            OLED_ShowFloatNum(3, 1, d);
            OLED_ShowFloatNum(4, 1, target);
            OLED_ShowNum(1, 10, command, 3);
        }
        if (command == 0x01)
        {
            HAL_TIM_Base_Start_IT(&htim2);
            time = 0;
            value = 0;
            command = 0;
        }
        if (command == 0x02)
        {
            HAL_TIM_Base_Stop_IT(&htim2);
            command = 0;
        }
        if (command == 0x03)
        {
            HAL_TIM_Base_Start_IT(&htim2);
            command = 0;
        }
        OLED_ShowNum(2, 10, l++, 3);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
    serial.interruptCallback(data);
    HAL_UART_Receive_IT(&huart1, &data, 1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    value = float(7*sin(time++)+7);
    serial.writeFrameData() << target << value;
    serial.sendFrame();
}