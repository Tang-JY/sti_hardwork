#include "stm32f4xx.h"
#include "delay.h"
#include "toolkit.h"
#include "cycle.h"
#include "usart1.h"
#include "usart2.h"

/**************************************/
/*             数据声明               */
/**************************************/
//无需同步的数据
const unsigned int dt = 15;//循环时间(ms)


//需要同步的数据
DataSync_RX_TypeDef rxData;//接收数据同步器
DataSync_TX_TypeDef txData;//发送数据同步器

/**************************************/
/*             结构体声明             */
/**************************************/
//_Graph_TypeDef graph_rpm_set, graph_rpm_current;
_DataController_TypeDef dc_vc_set, dc_vd_set;
_DataDisplay_TypeDef rpmc_display    , rpmd_display    , roll_display       , vc_display    , R_display;//实际反馈值
_DataDisplay_TypeDef roll_target_display, rpmc_set_display, rpmd_set_display, pwm0_display , pwm1_display;//中间变量
_StrDisplay_TypeDef msg;
_Key_TypeDef start_key;




/**************************************/
/*             初始化函数             */
/**************************************/
static void setup(void)
{	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	//必选初始化
	usart1_Init(115200);
	usart2_Init(115200,&txData,&rxData);
	delay_init(168);
	cycle_Init(dt);//循环周期初始化
	toolkit_Init(dt);//工具箱初始化
	
	
	//可选初始化
	dataController_Init();
	
	
	//配置函数
//	dataController_Struct_CFG(&dc_kp,  650, 50 , &(txData.kp), 10, "KP", encoder1, 0.0f, 10.0f );//数据显示器等纵向距离通常取30
//	dataController_Struct_CFG(&dc_ki,  650, 80 , &(txData.ki), 10, "KI", encoder1, 0.0f, 20.0f );
//	dataController_Struct_CFG(&dc_kd,  650, 110, &(txData.kd), 10, "KD", encoder1, 0.0f, 2.0f);
	
	
	dataDisplay_Struct_CFG(&vc_display  , 350, 50 , &(rxData.v   ), 10, "vc");
	dataDisplay_Struct_CFG(&R_display   , 350, 80 , &(rxData.R   ), 10, "R");
	dataDisplay_Struct_CFG(&roll_display, 350, 110, &(rxData.roll), 10, "roll");
	dataDisplay_Struct_CFG(&rpmc_display, 350, 170, &(rxData.rpmc), 10, "rpmc");
	dataDisplay_Struct_CFG(&rpmd_display, 350, 200, &(rxData.rpmd), 10, "rpmd");
	strDisplay_Struct_CFG(&msg, 350, 230, rxData.msg, 16, "msg");
	
	dataController_Struct_CFG(&dc_vc_set       , 150, 50 , &(txData.vc_set)     ,10, "vc_set", encoder0, -260.0f, 260.0f);
	dataController_Struct_CFG(&dc_vd_set       , 150, 80 , &(txData.vd_set)     ,10, "vd_set", encoder1, -30.0f , 30.0f);	
	dataDisplay_Struct_CFG(&roll_target_display, 150, 110, &(rxData.roll_target),10, "roll_set");
	dataDisplay_Struct_CFG(&rpmc_set_display   , 150, 170, &(rxData.rpmc_set)   ,10, "rpmc_set");
	dataDisplay_Struct_CFG(&rpmd_set_display   , 150, 200, &(rxData.rpmd_set)   ,10, "rpmd_set");
	dataDisplay_Struct_CFG(&pwm0_display       , 150, 230, &(rxData.pwm0  )     ,10, "pwm0"    );
	dataDisplay_Struct_CFG(&pwm1_display       , 150, 260, &(rxData.pwm1  )     ,10, "pwm1"    );
	
	
	
	
	key_Struct_CFG(&start_key, 580, 290, lock  , "Start");
	
	//数据修正
	txData.reg[0] = 0;
	txData.reg[1] = 0;
	txData.reg[2] = 0;
	txData.vc_set = 0;
	txData.vd_set = 0;
}




/**************************************/
/*             循环函数               */
/**************************************/
static void loop(void)
{	
//	u8 temp;

	//数据控制器
	dataController_KeyScan();
	//dataController_Draw(&dc_kp);
	//dataController_Draw(&dc_ki);
	//dataController_Draw(&dc_kd);
	dataController_Draw(&dc_vc_set);
	dataController_Draw(&dc_vd_set);
	
	//数据显示器
	dataDisplay_Draw(&vc_display);
	dataDisplay_Draw(&R_display);
	dataDisplay_Draw(&roll_display);
	dataDisplay_Draw(&rpmc_display);
	dataDisplay_Draw(&rpmd_display);
	
	dataDisplay_Draw(&roll_target_display);
	dataDisplay_Draw(&rpmc_set_display);
	dataDisplay_Draw(&rpmd_set_display);
	dataDisplay_Draw(&pwm0_display);
	dataDisplay_Draw(&pwm1_display);
	
	

	//字符串显示器
	strDisplay_Draw(&msg);
	
	//按键功能
	key_Scan_All();	
	if(key_Check(&start_key)){
		txData.reg[0] = 1;
	}else{
		txData.reg[0] = 0;
	}

	usart2_SendSync();//接收同步数据是实时的，但发送同步数据需要手动
	
}






/**************************************/
/*               主函数               */
/**************************************/
int main(void)
{
	setup();
	while(1){
		if(cycle_flag){
			cycle_flag = 0;
			loop();
		}
	}
}



