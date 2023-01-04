/********************************************************************
 * @file    main.c
 * @note    ʵ��RFID���������ܣ���T55557�����ж�д����
 *          ������,������125 kHz����ƵƵ���£�����RF/32��Bit Rate��Manchester���룬ʹ��Sequences Terminatorͬ���źš�
 * @device  fm33lc0xx������
 * @date    2022-11-15
 * @author  JBL
**********************************************************************/

/***********************************************
���ŷ��䣺
ʹ��GPTIM0 ��125khz �����ͨ��1. PB10(pin20),��������
ʹ��GPTIM1 �����벶��ͨ��1��PC0(pin25)����������
ʹ��ATM    ����ʱ��������ӣ��ж�
ʹ��GPIO��� LED�� PD6(pin44)
ʹ��GPIO��� BEEP. PC3(pin28)




���Լ�¼��

1. ��ʱ��У׼
   - ���125khz -ok
   - ���500ms���� -ok
2. ��������� - OK
3. ���ڵ��� - OK
4. �ܹ������ݣ�1.�������ݣ�2.������ȷ���ݣ�
5. �ܹ�д���� ��1.д������ ��2. д����ȷ���ݣ�

**********************************************/


#include "fm33lc0xx_fl.h"
#include "state.h"
#include "pll.h"
#include "uart.h"
#include "timer.h"
#include "beep.h"
#include "t5557.h"
#include "main.h"


extern uint8_t  RecSucc;  //���յ�β֡���ݱ�ʶλ 0-���ǣ�1-�ǣ�ͨ��β֡��ʶλ�жϣ��̶�0xBB
extern uint8_t  CardType; //������

//#define TEST_UART

#ifdef TEST_UART
//1- ����ͷ 2-���տ����� 3-�������ݳ���  4.��������(һֱ������)  5.���ڽ������ݳ���  6.����У����� 7.����У����� 8.������� 9.����β֡����0xBB����
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
*   @brief  �������
*   @note   note
**/
int main()
{
    /* ϵͳʱ������begin */
    
    //�ϵ�Ĭ��ʹ��8MHz RCHF�Ĳ���Ƶʱ����Ϊϵͳ��ʱ��
    // ����ϵͳ��ƵΪ64MHz,ʹ��RCHF-->PLL-->SYSCLK, ����С��20MHz�ƺ���������
    PLL_SelRCHFToPLL(FL_RCC_RCHF_FREQUENCY_8MHZ, 64 - 1);
    SystemCoreClockUpdate();	//����SystemCoreClock����ֵ
    
    //��ʱ����
    FL_Init();
    
    //led��ʼ��
    STATE_Init();
    STATE_Blink(3,500);
    //��������ʼ��
    BEEP_Init();
    
    /* ���ڳ�ʼ�� */
    UART_0_Init(9600);//����0����ӦDEBUG�������������PC
    AVL_PRINTF("RFIDTEST-V0.1.1\n");
    
    //��ʱ����ʼ��
    //GPTIM0_PWM_Init(1,512,1);//����Ƶ 64000khz��64000/arr = 125khz , arr =512,����T = 1/125khz = 8us,��ʱû�з�����������һ��ֱ�ߣ���Ϊ�Ƚ�ֵ=0��ռ�ձ�100%
    //FL_GPTIM_WriteCompareCH1(GPTIM0,256); //�ı�duty ���Ǹı�Ƚ�ֵ��ͨ�����ʸ�Ϊ RF/32 .�����Ƚ�ֵ�����ڲ��䣬����������ռ�ձ�50%
    GPTIM0_PWM_Init(64,8,4);//125khz
    
    GPTIM1_CaptureInit(64,0xFFFF);//1Mh����
    
    ATIM_Init(6400,5000);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms
    
    
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



