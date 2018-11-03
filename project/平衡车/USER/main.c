#include "stm32f4xx.h"

#include "usart1.h"
#include "usart2.h"
#include "MPU.h"
#include "delay.h"
#include "FaulhaberMotor.h"
#include "cycle.h"
#include "pid.h"
#include "signal.h"
#include "encoder.h"
#include <string.h>
#include <math.h>

#include "cpu_time.h"

/***************************************************/
/*                  数据声明                       */
/***************************************************/

//不需同步的数据
const float dt = 1.0f;


//轮半径
const float radius = 31.1f;

//电机环PID参数
float rpm0_set=0,rpm1_set=0;

float motor_p = 0.25f;
float motor_i = 0.25f;
float motor_d = 0.002f;


//加速度计环
const float roll_set_freq   = 0.8f;
float roll_set_last;

float mpu_p = 70.0f;
float mpu_i = 110.0f;
float mpu_d = 0.35f;

const float roll_set_offset = -0.85f;


//速度环
float v_feedback[2]={0.0f,0.0f};
//float vd_feedback[2]={0.0f,0.0f};
const float v_freq = 10.0f;
const float vc_set_freq = 10.0f;
float vc_set_last;

float vc_p = 0.06f;
float vc_i = 0.0488f;
float vc_d = 0.0003f;

/*
//位移环
float x_feedback[2]={0.0f,0.0f};
const float x_freq = 20.0f;
float x_set = 0.0f;//真实的目标值

float x_p = 3.5f;
float x_i = 6.0f;
float x_d = 0.6f;

*/


//需要同步的数据
extern DataSync_TX_TypeDef txData;
extern DataSync_RX_TypeDef rxData;

//其他中间变量
const float const_data0 = 3.14159f*radius/180.0f;
const float const_data1 = 2.0f*3.14159f/60.0f;
const float const_data2 = 1.0f/const_data1;

/***************************************************/
/*                功能结构体声明                   */
/***************************************************/

PID_TypeDef rpm0,rpm1;//电机速度
PID_TypeDef pid_roll;
PID_TypeDef pid_vc;
//PID_TypeDef pid_movement;








/***************************************************/
/*                初始化及配置                     */
/***************************************************/
static void setup()
{	
	//NVIC中断优先级分组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	//初始化
	cpuWorkTimeInit();
	delay_init(168);
	usart1_Init(115200);
	usart2_Init(115200);
	MPU_Init(921600);
	encoder_Init(dt);		
	FH_Motor_Init(50000, 0.0f, 99.5f);//电机初始化
	//cycle_Init(dt,mpu_Update,encoder_Update,NULL);//将三个函数绑定到定时器
	cycle_Init(dt);
	
	//配置
	pid_params_CFG(&rpm0,&motor_p,&motor_i,&motor_d,dt);
	pid_limits_CFG(&rpm0,10.0f,1.0f,200.0f);
	pid_data_CFG  (&rpm0,&rpm0_set,&(encoder0.rpm));
	
	pid_params_CFG(&rpm1,&motor_p,&motor_i,&motor_d,dt);
	pid_limits_CFG(&rpm1,10.0f,1.0f,200.0f);
	pid_data_CFG  (&rpm1,&rpm1_set,&(encoder1.rpm));
	
	pid_params_CFG(&pid_roll,&(mpu_p),&(mpu_i),&(mpu_d),dt);
	pid_limits_CFG(&pid_roll,13.0f,5.0f,200.0f);
	pid_data_CFG  (&pid_roll,&(txData.roll_target),&(mpu.roll));
	
	pid_params_CFG(&pid_vc,&vc_p,&vc_i,&vc_d,dt);
	pid_limits_CFG(&pid_vc,200.0f,15.0f,200.0f);
	pid_data_CFG(&pid_vc,&(rxData.vc_set),&(v_feedback[0]));
	
	/*
	pid_params_CFG(&pid_movement,&(x_p),&(x_i),&(x_d),dt);
	pid_limits_CFG(&pid_movement,10.0f,0.8f,100.0f);
	pid_data_CFG  (&pid_movement,&(x_set),&(x_feedback[0]));
	*/
	
	//siganal_CFG(&signal0,rect,&rpm0_set,&freq,&rpp,&bias,NULL,NULL,dt);
	

	//数值初始化
//	rxData.kp = 0.05f;
//	rxData.ki = 0.0f;
//	rxData.kd = 0.0f;
//	rxData.x_target = 0.0f;
	rxData.reg[0] = 0;
	rxData.reg[1] = 0;
	rxData.reg[2] = 0;
	
	
	txData.reg[0] = 0;
	txData.reg[1] = 0;
	

}

/***************************************************/
/*                      循环                       */
/***************************************************/
static void loop()
{	
	volatile float temp;
	volatile static vu32 cnt=0;//计数值
	
	//获取最新数据
	mpu_Update();
	encoder_Update();
	
	
	if(rxData.reg[0]){
		
//		if(rxData.reg[1]){
//			//若按下遥控移动按钮，则更新x目标值，否则保持
//			x_set = 4.684f*rxData.x_target;
//			txData.reg[0] = 0;//未到达目标标志位
//		}
		
		if(fabs(mpu.roll - roll_set_offset)>12.5f && txData.reg[0]==0){//角度进入危险区域
			cnt++;
			if(cnt*dt>300){//判断已经倒下
				txData.reg[0] = 1;
				
				//立即刹车，再释放
				txData.pwm0 = 0.0f;
				txData.pwm1 = 0.0f;
				FH_Motor_SetPWM(txData.pwm0,txData.pwm1);
				delay_ms(30*dt);//跳过30个周期
				cnt = 0;
			}
			
		}else if(fabs(mpu.roll - roll_set_offset)<9.0f&& txData.reg[0]==0){//直立状态
			
			if(cnt){
				cnt = 0;
			}
		}else if(fabs(mpu.roll - roll_set_offset)<9.0f&& txData.reg[0]==1){//从倒下恢复到直立状态
			cnt++;
			if(cnt*dt>700){//认为已经恢复平衡
				txData.reg[0] = 0;
				
			}
		}

/*
		//一级PID:控制车相对地面位移
		x_feedback[1] = x_feedback[1];
		
			//反馈值滤波
		temp =  ((encoder0.deg_rel + encoder1.deg_rel)/2.0f + mpu.roll)*3.14159f*radius/180.0f;//轮相对于地面位移=轮相对车位移+车相对地位移
		x_feedback[0] = (3.14159f*x_freq*2*dt)/(3.14159f*x_freq*2*dt+1000.0f)*temp + 1000.0f/(3.14159f*x_freq*2*dt+1000.0f)*x_feedback[1];
		
		vc_set_last = txData.vc_set;//保存上次速度目标值
		pid_Calc(&pid_movement);
		
		temp = pid_movement.u;
		
			//下一级目标值限辐
		if(temp>200.0f){
			temp = 200.0f;
		}else if(temp<-200.0f){
			temp = -200.0f;
		}
		
			//下一级目标值滤波
		txData.vc_set = (3.14159f*vc_set_freq*2*dt)/(3.14159f*vc_set_freq*2*dt+1000.0f)*temp + 1000.0f/(3.14159f*vc_set_freq*2*dt+1000.0f)*vc_set_last ;

*/		
		
		
		
		//二级PID：控制车相对地面速度
		v_feedback[1] = v_feedback[0];
			//反馈值滤波
		temp = ((encoder0.rpm + encoder1.rpm)*3.0f + mpu.wx)*const_data0;
		v_feedback[0] = (3.14159f*v_freq*2*dt)/(3.14159f*v_freq*2*dt+1000.0f)*temp + 1000.0f/(3.14159f*v_freq*2*dt+1000.0f)*v_feedback[1];	
		
//		vd_feedback[1] = vd_feedback[0];
//		temp = (encoder0.rpm - encoder1.rpm)*3.0f*const_data0;
//		vd_feedback[0] = (3.14159f*v_freq*2*dt)/(3.14159f*v_freq*2*dt+1000.0f)*temp + 1000.0f/(3.14159f*v_freq*2*dt+1000.0f)*vd_feedback[1];
		
		roll_set_last = txData.roll_target;//记录上次角度设定值
		pid_Calc(&pid_vc);
//		pid_Calc(&pid_vd);
			
			//下一级目标值限辐
		temp = pid_vc.u + roll_set_offset;
		if(temp > 2.8f + roll_set_offset){
			temp = 2.8f + roll_set_offset;
		}else if(temp < -2.8f+roll_set_offset){
			temp = -2.8f + roll_set_offset;
		}
			//下一级目标值滤波
		txData.roll_target = (3.14159f*roll_set_freq*2*dt)/(3.14159f*roll_set_freq*2*dt+1000.0f)*temp + 1000.0f/(3.14159f*roll_set_freq*2*dt+1000.0f)*roll_set_last ;
		
		
		
		//三级pid:控制倾角（不再滤波）
		pid_Calc(&pid_roll);
		txData.rpmc_set = -1.0f*pid_roll.u;
			//限辐
		if(txData.rpmc_set>120.0f){
			txData.rpmc_set = 120.0f;
		}else if(txData.rpmc_set < -120.0f){
			txData.rpmc_set = -120.0f;
		}		
		
/*	
		if(rxData.reg[2]){
		//考虑转动半径
			if(rxData.R < 25.0f){
				rxData.R = 25.0f;
			}

			//转动方向
			if(!rxData.reg[3]){
				txData.rpmd_set = -32.7f*txData.rpmc_set/rxData.R;
			}else{
				txData.rpmd_set =  32.7f*txData.rpmc_set/rxData.R;
			}
			
		}else{
			txData.rpmd_set = 0.0f;
		}
*/
		
		txData.rpmd_set = rxData.vd_set*const_data2;
		
		if(txData.rpmd_set >35.0f){
			txData.rpmd_set = 35.0f;
		}else if(txData.rpmd_set <-35.0f){
			txData.rpmd_set = -35.0f;
		}
		
		rpm0_set = txData.rpmc_set + (txData.rpmd_set/2.0f);
		rpm1_set = txData.rpmc_set - (txData.rpmd_set/2.0f);
		
		//四级pid:控制电机相对车体速度
		pid_Calc(&rpm0);	
		pid_Calc(&rpm1);
		
		txData.pwm0 = rpm0.ui;
		txData.pwm1 = rpm1.ui;
		
		FH_Motor_SetPWM(txData.pwm0,txData.pwm1);
		
	}else{
		
		FH_Motor_SetPWM(0,0);
	}
	
	
	txData.rpmc = encoder0.rpm + encoder1.rpm;
	txData.rpmd = encoder0.rpm - encoder1.rpm;
	txData.roll = mpu.roll;
	txData.v    = v_feedback[0]/4.684f;
//	txData.R    = fabs(32.7f*txData.rpmd/txData.rpmc);
//	txData.x    = x_feedback[0]/4.684f;
	
/*	if(fabs(x_feedback[0] - x_set) <1.0f ){
		txData.reg[0] = 1;//到达目标
	}
*/
//	printf("rpmc:%.2f rpmd:%.2f R:%.2f\r\n",txData.rpmc,txData.rpmd,txData.R);
	usart2_SendSync();
	

	
	//printf(" %.2f,%.2f\r\n",rpm0_set, encoder0.rpm);
	
//	mpu_Update();
//	encoder_Update();
}




int main(void)
{
	setup();
	while(1){
		if(cycle_flag){
			cycle_flag = 0;
			cpuWorkStart();
			loop();
			cpuWorkEnd();
		}		
	}
}




