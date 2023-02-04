//
// Created by fazhehy on 2023/2/4.
//

#include "common_inc.h"
#include "Serial.h"

Serial serial(&huart1);
uint8_t data;

void Main()
{
    HAL_UART_Receive_IT(&huart1, &data, 1);
    for(;;)
    {

    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
    serial.interruptCallback(data);
    HAL_UART_Receive_IT(&huart1, &data, 1);
}
