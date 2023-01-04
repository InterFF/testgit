/********************************************************************
 * @file    main.c
 * @note    实现RFID读卡器功能，对T55557卡进行读写操作
 *          读卡器,工作在125 kHz的射频频率下，采用RF/32的Bit Rate，Manchester编码，使用Sequences Terminator同步信号。
 * @device  fm33lc0xx开发板
 * @date    2022-11-15
 * @author  JBL
**********************************************************************/

/***********************************************
引脚分配：
使用GPTIM0 做125khz 输出，通道1. PB10(pin20),数字外设
使用GPTIM1 做输入捕获，通道1。PC0(pin25)，数字外设
使用ATM    做定时输出，闹钟，中断
使用GPIO输出 LED。 PD6(pin44)
使用GPIO输出 BEEP. PC3(pin28)




调试记录：

1. 定时器校准
   - 输出125khz -ok
   - 输出500ms闹钟 -ok
2. 蜂鸣器输出 - OK
3. 串口调试 - OK
4. 能够读数据（1.读出数据，2.读出正确数据）
5. 能够写数据 （1.写入数据 ，2. 写入正确数据）

**********************************************/


#include "fm33lc0xx_fl.h"
#include "state.h"
#include "pll.h"
#include "uart.h"
#include "timer.h"
#include "beep.h"
#include "t5557.h"
#include "main.h"


extern uint8_t  RecSucc;  //接收到尾帧数据标识位 0-不是，1-是，通过尾帧标识位判断，固定0xBB
extern uint8_t  CardType; //卡类型

//#define TEST_UART

#ifdef TEST_UART
//1- 接收头 2-接收卡类型 3-接收数据长度  4.接收数据(一直接收中)  5.大于接收数据长度  6.接收校验完成 7.接收校验出错 8.接收完成 9.接收尾帧不是0xBB出错
extern volatile uint8_t test_st;
extern volatile uint8_t test_sp;
extern uint8_t  Num;

void testUart(void)
{
        if(RecSucc)
        {
            AVL_PRINTF("------------------------------------------------------\n\r\n\r\n\r");
            NIP_PRINTF("%d\r\n",test_sp);
            NIP_PRINTF("%d\r\n",Num);
            NIP_PRINTF("%d\r\n",test_st);
            test_st = 0;
            test_sp = 0;
            uartStatusInit();
            
        }
        
        NIP_PRINTF("%d\r\n",test_st);
}
#endif

/*
*   @brief  程序入口
*   @note   note
**/
int main()
{
    /* 系统时钟设置begin */
    
    //上电默认使用8MHz RCHF的不分频时钟作为系统主时钟
    // 设置系统主频为64MHz,使用RCHF-->PLL-->SYSCLK, 设置小于20MHz似乎不起作用
    PLL_SelRCHFToPLL(FL_RCC_RCHF_FREQUENCY_8MHZ, 64 - 1);
    SystemCoreClockUpdate();	//更新SystemCoreClock变量值
    
    //延时函数
    FL_Init();
    
    //led初始化
    STATE_Init();
    STATE_Blink(3,500);
    //蜂鸣器初始化
    BEEP_Init();
    
    /* 串口初始化 */
    UART_0_Init(9600);//串口0，对应DEBUG调试输出，连接PC
    AVL_PRINTF("RFIDTEST-V0.1.1\n");
    
    //定时器初始化
    //GPTIM0_PWM_Init(1,512,1);//不分频 64000khz。64000/arr = 125khz , arr =512,周期T = 1/125khz = 8us,此时没有方波产生，是一条直线，因为比较值=0，占空比100%
    //FL_GPTIM_WriteCompareCH1(GPTIM0,256); //改变duty 就是改变比较值。通信速率改为 RF/32 .给定比较值，周期不变，产生方波，占空比50%
    GPTIM0_PWM_Init(64,8,4);//125khz
    
    GPTIM1_CaptureInit(64,0xFFFF);//1Mh计数
    
    ATIM_Init(6400,5000);//10Khz的计数频率，计数到5000为500ms
    
    
    //AA 02 07 85 0A 55 AA AA AA AA DF BB
    while(1)
    {
        #ifdef TEST_UART
        testUart();
        #endif
        
        //FL_DelayMs(100);
        
       MyPrintf("well\n");
//       STATE_TOG();
//       
//       
//       //testdata
//       RecSucc = 1;
//       CardType = 0x02;

        if(RecSucc)
        {
             switch (CardType)
			 {
			     case 0x02: t5557Handler(); break;
				 case 0x01: break;
				 default :  break;
			 }

			 uartStatusInit();
        }
        
//        
        FL_DelayMs(1000);
    }
}



