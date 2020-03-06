#include "bc26.h"
#include "string.h"
#include "usart.h"
#include "wdg.h"
#include "led.h"

char *strx,*extstrx;
extern unsigned char RxBuffer[255],RxCounter;
BC26 BC26_Status;
unsigned char socketnum=0;//当前的socket号码

// 阿里云的三元素,在设备里面都可以查询到
#define ProductKey      "a1LLZXC0Ziz"
#define DeviceName      "instance1"
#define DeviceSecret    "Krz8a9g1qWVGWtrP453s9x5p6AALKNwX"
#define PubTopic        "/a1LLZXC0Ziz/instance1/user/update"
#define SubTopic        "/a1LLZXC0Ziz/instance1/user/get"
// #define PubTopic        "/sys/a1LLZXC0Ziz/instance1/thing/event/property/post"
// #define SubTopic        "/sys/a1LLZXC0Ziz/instance1/thing/service/property/set"

void Clear_Buffer(void) //清空缓存
{
    u8 i;
    Uart3_SendStr(RxBuffer);
    for (i = 0; i < 100; i++)
        RxBuffer[i] = 0; //缓存
    RxCounter = 0;
    IWDG_Feed(); //喂狗
}

void BC26_Init(void)
{
    printf("AT\r\n"); 
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
    Clear_Buffer();	
    while(strx==NULL)
    {
        Clear_Buffer();	
        printf("AT\r\n"); 
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
    }
    printf("AT+CFUN=1\r\n");//获取卡号，类似是否存在卡的意思，比较重要。
    delay_ms(300);
    printf("AT+CIMI\r\n");//获取卡号，类似是否存在卡的意思，比较重要。
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"460");//返460，表明识别到卡了
    Clear_Buffer();	
    while(strx==NULL)
    {
        Clear_Buffer();	
        printf("AT+CIMI\r\n");//获取卡号，类似是否存在卡的意思，比较重要。
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"460");//返回OK,说明卡是存在的
    }
        printf("AT+CGATT=1\r\n");//激活网络，PDP
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//返OK
        Clear_Buffer();	
        printf("AT+CGATT?\r\n");//查询激活状态
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"+CGATT: 1");//返1
        Clear_Buffer();	
		while(strx==NULL)
		{
            Clear_Buffer();	
            printf("AT+CGATT?\r\n");//获取激活状态
            delay_ms(300);
            strx=strstr((const char*)RxBuffer,(const char*)"+CGATT: 1");//返回1,表明注网成功
		}
		printf("AT+CESQ\r\n");//查看获取CSQ值
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"+CESQ");//返回CSQ
		if(strx)
			{
				BC26_Status.CSQ=(strx[7]-0x30)*10+(strx[8]-0x30);//获取CSQ
				if((BC26_Status.CSQ==99)||((strx[7]-0x30)==0))//说明扫网失败
				{
					while(1)
					{
                        BC26_Status.netstatus=0;
						Uart1_SendStr("信号搜索失败，请查看原因!\r\n");
                        RESET=1;//拉低
                        delay_ms(300);
                        delay_ms(300);	
                        RESET=0;//复位模块
						delay_ms(300);//没有信号就复位
                        
					}
				}
             else
             {
                 BC26_Status.netstatus=1;
              }
                
            }
              Clear_Buffer();	
}


void BC26_ConUDP(void)
{
	uint8_t i;
	printf("AT+QSOCL=0\r\n");//关闭socekt连接
	delay_ms(300);
    IWDG_Feed();//喂狗
}


void BC26_ConTCP(void)
{
		uint8_t i;
	printf("AT+QICLOSE=0\r\n");//关闭socekt连接
	delay_ms(300);
    Clear_Buffer();
    IWDG_Feed();//喂狗
}


void BC26_CreateTCPSokcet(void)//创建sokcet
{

    printf("AT+QIOPEN=1,0,\"TCP\",\"47.99.80.89\",14269,1234,1\r\n");//创建连接TCP,输入IP以及服务器端口号码 ,采用直接吐出的方式
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"+QIOPEN: 0,0");//检查是否登陆成功
 	while(strx==NULL)
		{
            strx=strstr((const char*)RxBuffer,(const char*)"+QIOPEN: 0,0");//检查是否登陆成功
            delay_ms(100);
		}  
     Clear_Buffer();	
    
}


void BC26_Senddata(uint8_t *len,uint8_t *data)//字符串形式
{
    printf("AT+QSOSEND=0,%s,%s\r\n",len,data);
}


void BC26_Senddatahex(uint8_t *len,uint8_t *data)//发送十六进制数据
{
    printf("AT+QISENDEX=0,%s,%s\r\n",len,data);
        delay_ms(300);
 	while(strx==NULL)
		{
            strx=strstr((const char*)RxBuffer,(const char*)"SEND OK");//检查是否发送成功
            delay_ms(100);
		}  
     Clear_Buffer();	
}


void BC26_RECData()
{
    char i;
    static char nexti;
    strx=strstr((const char*)RxBuffer,(const char*)"+QSONMI");//返回+QSONMI，表明接收到UDP服务器发回的数据
    if(strx)
    {
       
        BC26_Status.Socketnum=strx[8];//编号
      //  BC26_Status.reclen=strx[10];//长度,低于10个内的
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)",");//获取到第一个逗号
        for(i=0;;i++)
        { 
            if(strx[i+1]==',')
            break;
            BC26_Status.recdatalen[i]=strx[i+1];//获取数据长度
        }
        strx=strstr((const char*)(strx+1),(const char*)",");//获取到第二个逗号
        for(i=0;;i++)
        {
            if(strx[i+1]==0x0d)
            break;
            BC26_Status.recdata[i]=strx[i+1];//获取数据内容
        }
            Clear_Buffer();      
    }
}


void BC26_RECTCPData()
{
    char i;
    static char nexti;
    strx=strstr((const char*)RxBuffer,(const char*)"+QIURC:");//返回+QIURC:，表明接收到TCP服务器发回的数据
    if(strx)
    {
        Clear_Buffer();      
    }
}


void BC26_ChecekConStatus(void)
{
    char i;
    static char nexti;
    strx=strstr((const char*)RxBuffer,(const char*)"socket_t is NULL");//表明电信强制断开连接
    if(strx)
    {
        BC26_CreateTCPSokcet();//重新创建一个SOCKET连接
        Clear_Buffer();	
       
    }
}


void MQTT_Init(void)
{
    printf("AT+QMTCFG=\"aliauth\",0,\"%s\",\"%s\",\"%s\"\r\n",ProductKey,DeviceName,DeviceSecret);
    delay_ms(300);
    printf("AT+QMTOPEN=0,\"139.196.135.135\",1883\r\n");//通过TCP方式去连接MQTT阿里云服务器 
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"+QMTOPEN: 0,0");//看下返回状态
    while(strx==NULL)
    {
      strx=strstr((const char*)RxBuffer,(const char*)"+QMTOPEN: 0,0");//确认返回值正确
    }
    Clear_Buffer(); 
    printf("AT+QMTCONN=0,\"clientExample_2020\"\r\n");//去登录MQTT服务器
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"+QMTCONN: 0,0,0");//看下返回状态
    while(strx==NULL)
    {
        strx=strstr((const char*)RxBuffer,(const char*)"+QMTCONN: 0,0,0");//看下返回状态
    }
    Clear_Buffer(); 
    /*
    printf("AT+QMTSUB=0,1,\"mzh_m26\",0\r\n");//订阅个主题
        delay_ms(500);
    strx=strstr((const char*)RxBuffer,(const char*)"+QMTSUB: 0,1,0,0");//订阅成功
  while(strx==NULL)
    {
        strx=strstr((const char*)RxBuffer,(const char*)"+QMTSUB: 0,1,0,0");//订阅成功
    }
    Clear_Buffer(); 
    */
}


void aliyunMQTT_PUBdata(u8 temp,u8 humi)
{
    u8 t_payload[200],len;
    printf("AT+QMTPUB=0,0,0,0,%s\r\n",PubTopic);    //发布主题
    delay_ms(300);
    // 将temp,humi放入需要publish的字符串中
    len=Mqttaliyun_Savedata(t_payload,temp,humi);
    t_payload[len]=0;
    // 主要 向串口发送Json
    printf("%s",t_payload);
    // 发送Ctrl+Z终止
    while((USART1->SR&0X40)==0)
        ; //循环发送,直到发送完毕   
    USART1->DR = (u8) 0x1A;
    delay_ms(100);
    // 看下返回状态
    strx=strstr((const char*)RxBuffer,(const char*)"+QMTPUB: 0,0,0");
    // 看下返回状态
    while(strx==NULL){
        strx=strstr((const char*)RxBuffer,(const char*)"+QMTPUB: 0,0,0"); 
    }
    Clear_Buffer(); 
}


//访问阿里云需要提交JSON数据
u8 Mqttaliyun_Savedata(u8 *t_payload,u8 temp,u8 humi)
{
	int err;
	uint16_t pkt_id = 1;
    char led1status1,led1status2;
    char json[] = "{\"id\":\"26\",\"version\":\"1.0\",\"params\":{\"CurrentTemperature\":{\"value\":50,\"time\": 1524448722000},\"RelativeHumidity\":{\"value\":50,\"time\": 1524448722000}},\"method\":\"thing.event.property.post\"}";
    // char json[] = "{\"id\":\"26\",\"version\":\"1.0\",\"params\":{\"CurrentTemperature\":{\"value\":%d,\"time\": 1524448722000},\"RelativeHumidity\":{\"value\":%d,\"time\": 1524448722000}},\"method\":\"thing.event.property.post\"}";
    // char json[]="{\"datastreams\":[{\"id\":\"location\",\"datapoints\":[{\"value\":{\"lon\":%2.6f,\"lat\":%2.6f}}]}]}";
    char t_json[200];
    int payload_len;
    unsigned short json_len;

    //将字符串json写入t_json
    sprintf(t_json, json, temp, humi);
    payload_len =  strlen(t_json)/sizeof(char);
    //计算t_json的字节数
    json_len = strlen(t_json)/sizeof(char);
    //将t_json复制进t_payload
    memcpy(t_payload, t_json, json_len);
    return json_len;
}

