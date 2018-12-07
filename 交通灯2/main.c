#include <reg52.h>
#include <intrins.h>

char code num[]={0x3F,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};
char current[]={0x00,0x00,0x00};
static int sec,i,cnt;
sbit LED1=P2^1;
sbit LED2=P2^2;
sbit LED3=P2^3;
sbit LED4=P2^4;
sbit ADDR0=P3^2;
sbit ADDR1=P3^3;
sbit ADDR2=P3^4;
sbit ENLED=P2^0;

sbit SDA=P3^6;
sbit SCL=P3^5;

//内部数据定义
#define IIC_Add 0xB8    //AM2320地址
#define IIC_RX_Length 15

unsigned char IIC_TX_Buffer[]={0x03,0x00,0x04}; //读温湿度命令
unsigned char IIC_RX_Buffer[IIC_RX_Length] = {0x00};//读回的温湿度
unsigned char str[12];
unsigned char pm[7];//pm2.5数据
unsigned char Uart_RX_Buffer[30] = {0x00};
unsigned char *String;
unsigned char WR_Flag;
unsigned char RxdByte;
unsigned long result;


void LEDswitch();
void timer(int s);
void Ack(void);
void NoAck(void);
void Clear_Data (void);
void Waken(void);
bit WriteNByte(unsigned char sla,unsigned char *s,unsigned char n);
bit ReadNByte(unsigned char Sal, unsigned char *p,unsigned char n);
void delay1ms(unsigned int t);
char WaitingTime(unsigned char val);
extern void InitLcd1602();
extern void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str);
extern void LcdAreaClear(unsigned char x, unsigned char y, unsigned char len);
extern void PutNumToString(unsigned char str[] ,unsigned int num);
extern unsigned char I2CReadACK();
extern unsigned char I2CReadNAK();
extern bit I2CWrite(unsigned char dat);
extern void LongToString(unsigned char *str, signed long dat);
extern char GetADCValue(unsigned char chn);
extern void ConfigUART(int a);
extern int getresult(char pm[]);

int main()
{
	EA = 1;
InitLcd1602();
	  TMOD=0x01;
	TH0 = 0xfc;
	TL0=0x67;
	ET0 = 1;
	TR0=1;
	ENLED=0;
	ConfigUART(2400);  
	LcdShowStr(0,1,"next time");
while(1){
    LEDswitch();
		timer(WaitingTime(GetADCValue(0)));
}
}

void timer(int s)
{
	sec=s;
	while(sec>0)
	{
		current[0]=num[sec%10];
		current[1]=num[sec/10%10];
		current[2]=num[sec/100%10];
		if(sec%4==0)
			{
			Clear_Data(); // 清除收到数据
	WR_Flag = 0;
	Waken();	  // 唤醒传感器
  WriteNByte(IIC_Add,IIC_TX_Buffer,3); //发送读指令
	delay1ms(2);    
    ReadNByte(IIC_Add,IIC_RX_Buffer,8);//读返回数据
	SCL = 1; SDA = 1;	//确认释放总线
	PutNumToString(str ,IIC_RX_Buffer[4]*256+IIC_RX_Buffer[5]);
	LcdAreaClear(0,0,20);
	LcdShowStr(0,0,"temperature:");
	if(str[0]<53&&str[0]>48)LcdShowStr(12,0,str);
		}
		else if(sec%4==2)
		{
			LcdAreaClear(0,0,20);
			LcdShowStr(0,0,"pm2.5:");
			LongToString(str,result);
			LcdShowStr(7,0,str);
		}
		if(sec%4==1||sec%4==3)
		{
			LcdAreaClear(9,1,3);
			LongToString(str,WaitingTime(GetADCValue(0)));
			LcdShowStr(9,1,str);
		}
	}
}

void interrupttimer() interrupt 1
{
	TH0 = 0xfc;
	TL0=0x67;
	cnt++;
	if(cnt>=1000)
	{
		cnt=0;
	  sec--;
	}
	P1=0x00;
	
	switch(i)
	{
		case 0:ADDR2=0;ADDR1=0;ADDR0=0;i++;P1=current[2];break;
		case 1:ADDR2=0;ADDR1=0;ADDR0=1;i++;P1=current[1];break;
		case 2:ADDR2=0;ADDR1=1;ADDR0=0;i++;P1=current[0];break;
    default : break;
	}
	if(i>2)
	{
		i=0;
	}
}

void LEDswitch(){
	if(LED1){
		LED1=0;LED2=1;LED3=1;LED4=0;
	}
	else{
		LED1=1;LED2=0;LED3=0;LED4=1;
	}
}


//iic

 
void delay10us(void) //这个延时函数 要大于5US以上
{
  _nop_(); _nop_(); _nop_(); 
  _nop_(); _nop_(); _nop_(); 
}
 
void delay1ms(unsigned int t)
{
  unsigned int i;
  unsigned int j;
  for(j=t;j>0;j--)
   for(i=124;i>0;i--);  
}
//**********************************************
//送起始位 sda=1->0
void I2C_Start()
{
  SDA=1;
  SCL=1;
  delay10us();
  SDA=0;
  delay10us();
  SCL=0; 
}
//************************************************
//送停止位 sda=0->1
void I2C_Stop()
{
   SDA=0;
   delay10us();
   SCL=1;
   delay10us();
   SDA=1;
}
//************************************************
//主应答(包含ack:sda=0和no_ack:sda=0)
void Ack(void)
{  //设置SDA 口为输出
   SDA=0;
   SCL=0;
   delay10us();
   SCL=1;
   delay10us();	
   SCL=0;
   SDA=1;
}

void NoAck(void)
{  //设置SDA 口为输出
   SDA=1;  
   SCL=0;
   delay10us();
   SCL=1;
   delay10us();
   SDA=1;
   SCL=0;
}

// 检测 SDA是否回ACK
bit Test_Ack()
{  //设置SDA 口为输入
   bit ACK_Flag=0;
   SCL=0;
   SDA=1;    
   delay10us();
   SCL=1;
   delay10us();
   if(SDA==0)
     ACK_Flag = 1;
   else 
     ACK_Flag = 0;
   SCL=0;
   return ACK_Flag;
}

//*************************************************
//字节发送程序
//发送c(可以是数据也可是地址)，送完后接收从应答
//不考虑从应答位
void SendData(unsigned char buffer)
{
   unsigned char BitCnt=8;//一字节8位
   //设置SDA 口为输出
   do
   {
 	  SCL=0;
	  delay10us();
      if((buffer&0x80)==0) //判断最高位是0还是1
        SDA=0;
      else
        SDA=1;
      SCL=1;
	  delay10us();
      buffer=buffer<<1;//将buffer中的数据左移一位
      BitCnt--;
   }
   while(BitCnt);
   SCL=0;        
}
//**************************************************
//字节接收程序
//接收器件传来的数据，此程序应配合|主应答函数|i2c_ack_main()使用
//return: uchar型1字节
unsigned char ReceiveData()
{
  unsigned char BitCnt=8,IIC_RX_Data=0;
  unsigned char temp=0;
  SDA=1;           //读入数据 设置SDA 口为输入
  do
  {
     SCL=0;
	 delay10us();  
	 IIC_RX_Data=_crol_(IIC_RX_Data,1);   //数据左移一位
	 BitCnt--;	  
	 SCL=1;
	 delay10us();
     if(SDA==1)
       IIC_RX_Data = IIC_RX_Data|0x01;  //低位置1
     else
       IIC_RX_Data = IIC_RX_Data&0x0fe; //低位清0	    
   }
   while(BitCnt);
   SCL=0;
   return IIC_RX_Data;
}
//***************************************************
bit WriteNByte(unsigned char sla,unsigned char *s,unsigned char n)
{
   unsigned char i;
   
   I2C_Start();  //启动I2C
   SendData(sla);//发送器件地址
   if(!Test_Ack())
   {	
      WR_Flag = 1;
	  return(0);
   }
   for(i=0;i<n;i++)//写入8字节数据
   {
      SendData(*(s+i));
	  if(!Test_Ack())
	  {
	    WR_Flag = 1;
		return(0);
	  }
   }
   I2C_Stop();
   return(1);
}
bit ReadNByte(unsigned char Sal, unsigned char *p,unsigned char n)
{
  unsigned char i;
  I2C_Start();    // 启动I2C
  SendData((Sal)| 0x01); //发送器件地址
  if(!Test_Ack())
  {
  	WR_Flag = 1;
	return(0);
  }
  delay10us();  
  delay10us();
  delay10us(); 
        
  for(i=0;i<n-1;i++)  //读取字节数据
  {
     *(p+i)=ReceiveData(); //读取数据
     Ack(); 
  }
  *(p+n-1)=ReceiveData();        
  NoAck();
  I2C_Stop(); 
  return(1);	 
}

void Waken(void)
   {
    I2C_Start();       // 启动I2C
    SendData(IIC_Add); // 发送器件地址
    Test_Ack();	       
    delay1ms(2);       // 至少延时1个Ms
    I2C_Stop();	
   }
void Clear_Data (void)
  {
	    int i;
	    for(i=0;i<IIC_RX_Length;i++)
	     {
	     IIC_RX_Buffer[i] = 0x00;
	     }//接收数据清零
 }

 /* UART中断服务函数 */
void InterruptUART() interrupt 4
{
    if (RI)  //接收到字节
    {
        RI = 0;  //手动清零接收中断标志位
        RxdByte = SBUF;  //接收到的数据保存到接收字节变量中
    }
		pm[i]=RxdByte;
		i++;
if(RxdByte==0xaa)i=1;
if(i==6&&(pm[5]==pm[1]+pm[2]+pm[3]+pm[4]))result=getresult(pm);
			  if (TI)  //字节发送完毕
    {
        TI = 0;  //手动清零发送中断标志位
    }
}

//等待时间
char WaitingTime(unsigned char val)
{
	val = val*25/255;
	if(val>3)return 10;
	else if(val>1&&val<=3)return 8;
	return 5;
}