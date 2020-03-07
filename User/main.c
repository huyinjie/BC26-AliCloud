#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "math.h"			
#include "stdio.h"
#include "stm32f10x_flash.h"
#include "stdlib.h"
#include "string.h"
#include "wdg.h"
#include "timer.h"
#include "stm32f10x_tim.h"
#include "bc26.h"	 
#include "dht11.h"
extern char  RxBuffer[100],RxCounter;
extern unsigned char uart1_getok;
extern char RxCounter1,RxBuffer1[100];
extern unsigned char Timeout,restflag;

void OPEN_BC26(void)
{
    char *strx;

    printf("AT\r\n");
    delay_ms(300);
    strx = strstr((const char *)RxBuffer, (const char *)"OK"); //返回OK
    printf("AT\r\n");
    delay_ms(300);
    strx = strstr((const char *)RxBuffer, (const char *)"OK"); //返回OK
    IWDG_Feed();                                               //喂狗
    if (strx == NULL)
    {
        PWRKEY = 1; //拉低
        delay_ms(300);
        delay_ms(300);
        delay_ms(300);
        delay_ms(300);
        PWRKEY = 0;  //拉高正常开机
        IWDG_Feed(); //喂狗
    }
    printf("AT\r\n");
    delay_ms(300);
    IWDG_Feed();                                               //喂狗
    strx = strstr((const char *)RxBuffer, (const char *)"OK"); //返回OK
    if (strx == NULL)                                          //如果设备休眠了，就复位模块
    {
        RESET = 1; //拉低
        delay_ms(300);
        delay_ms(300);
        RESET = 0; //复位模块
    }
    printf("ATE0&W\r\n"); //关闭回显
    delay_ms(300);
    LED = 0;
    IWDG_Feed(); //喂狗
    // Disconnect a Client from MQTT Server
    printf("AT+QMTDISC=0\r\n");
    delay_ms(300);
    // Close a Network for MQTT Client
    printf("AT+QMTCLOSE=0\r\n");
    delay_ms(300);
}

/***
对于电信卡而言，由于电信对IP的限制，TCP发送也会存在限制，所以TCP如果不是绑定IP也是会发送失败，对于移动而言不影响使用。建议移动客户进行实验
***/

//适用于NB版本
int main(void)
{
    u8 temp, humi;
    u8 sendata[] = "727394ACB8221234";
    delay_init();             //延时函数初始化
    NVIC_Configuration();     //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    LED_Init();               //初始化与LED连接的硬件接口
    BC26CTR_Init();           //初始化BC26的PWR与RESET引脚
    uart_init(115200);        //串口1初始化，可连接PC进行打印模块返回数据
    uart3_init(9600);       //初始化和GPRS连接串口
                              // IWDG_Init(7,625);     //8S一次
                              //  while(DHT11_Init())
                              // {}
    OPEN_BC26();              //对BC26开机
    TIM3_Int_Init(999, 7199); //100ms更新一次
    TIM4_Int_Init(999, 7199); //100ms更新一次
    BC26_Init();              //对设备初始化
    MQTT_Init();
    while (1)
    {
        //  DHT11_Read_Data(&temp,&humi);     //读取温湿度数据
        temp = 32;
        humi = 67;
        aliyunMQTT_PUBdata(temp, humi);
        delay_ms(500);
        IWDG_Feed(); //喂狗
    }
}
