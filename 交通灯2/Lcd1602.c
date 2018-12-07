
#include <reg52.h>

#define LCD1602_DB  P0
sbit LCD1602_RS = P2^7;
sbit LCD1602_RW = P2^6;
sbit LCD1602_E  = P2^5;

void LcdWaitReady()
{
    unsigned char sta;
    
    LCD1602_DB = 0xFF;
    LCD1602_RS = 0;
    LCD1602_RW = 1;
    do {
        LCD1602_E = 1;
        sta = LCD1602_DB; 
        LCD1602_E = 0;
    } while (sta & 0x80); 
}
void LcdWriteCmd(unsigned char cmd)
{
    LcdWaitReady();
    LCD1602_RS = 0;
    LCD1602_RW = 0;
    LCD1602_DB = cmd;
    LCD1602_E  = 1;
    LCD1602_E  = 0;
}

void LcdWriteDat(unsigned char dat)
{
    LcdWaitReady();
    LCD1602_RS = 1;
    LCD1602_RW = 0;
    LCD1602_DB = dat;
    LCD1602_E  = 1;
    LCD1602_E  = 0;
}
/* 设置光标位置，(x,y)-对应屏幕上的字符坐标 */
void LcdSetCursor(unsigned char x, unsigned char y)
{
    unsigned char addr;
    
    if (y == 0)  
        addr = 0x00 + x;  
    else
        addr = 0x40 + x;  
    LcdWriteCmd(addr | 0x80);  
}
/* 在液晶上显示字符串，(x,y)-对应屏幕上的起始坐标，str-字符串指针 */
void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str)
{
    LcdSetCursor(x, y);   //设置起始地址
    while (*str != '\0')  //连续写入字符串数据，直到检测到结束符
    {
        LcdWriteDat(*str++);
    }
}
/* 区域清除，清除从(x,y)坐标起始的len个字符位 */
void LcdAreaClear(unsigned char x, unsigned char y, unsigned char len)
{
    LcdSetCursor(x, y);   //设置起始地址
    while (len--)         //连续写入空格
    {
        LcdWriteDat(' ');
    }
}
/* 初始化1602液晶 */
void InitLcd1602()
{
    LcdWriteCmd(0x38);  
    LcdWriteCmd(0x0C); 
    LcdWriteCmd(0x06); 
    LcdWriteCmd(0x01);  //清屏
}

void PutNumToString(unsigned char str[] ,unsigned int num)
{
	unsigned char a[4],i;
	a[3] = '0'+num%10;
	a[2] = '.';
	a[1] = '0'+num/10%10;
	a[0] = '0'+num/100%10;
	 for(i=0;i<4;i++)
	{
		str[i]=a[i];
	} 
}