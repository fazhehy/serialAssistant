//
// Created by 86176 on 2022/5/7.
//

#ifndef OLED_OLED_H
#define OLED_OLED_H

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowPicture(uint8_t arr[]);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowFloatNum(uint8_t Line, uint8_t Column, float Num);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

#endif //OLED_OLED_H
