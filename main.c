/*Base register adddress header file*/
#include "stm32l1xx.h"
/*Library related header files*/
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_tim.h"
#include "stm32l1xx_ll_adc.h"
#include "stdlib.h"

#define TIMx_PSC 32 //define prescaler
#define TIMx_ARR 20000 //define ARR

#define LDR1_PIN LL_GPIO_PIN_4
#define LDR2_PIN LL_GPIO_PIN_5
#define SERVO_PIN LL_GPIO_PIN_6

void SystemClock_Config(void);
void TIM_Base_Config(void); //function prototype
void TIM_OC_GPIO_Config(void); //function prototype
void TIM_OC_Config(void); //function prototype

void ADC_Init (void);
void ADC_Enable (void);
void ADC_Start (int channel);
void ADC_WaitForConv (void);
uint16_t ADC_GetVal (void);
void ADC_Disable (void);

int CCR = 1100; // initial position of the Horizontal movement controlling servo motor
int tolerance = 200; // allowable tolerance setting - so solar servo motor isn't constantly in motion
int ldr_diff;
uint16_t first_ldr;
uint16_t second_ldr;

int main()
{
	SystemClock_Config();
	TIM_OC_Config();
	ADC_Init();
	ADC_Enable();
	
	while(1){	
		ADC_Start(4);
		ADC_WaitForConv();
		first_ldr = ADC_GetVal();
		ADC1->DR = 0;
		ADC_Start(5);
		ADC_WaitForConv ();
		second_ldr = ADC_GetVal();
		
		if(abs(first_ldr-second_ldr)> tolerance){
      if(first_ldr > second_ldr && CCR<2000)	//CCR 2000 = 180 degree
				CCR += 5;	// CCR+-5 will control servo motor to rotate 0.9 degree
			else if(first_ldr < second_ldr && CCR>500) //CCR 1000 = 0 degree
				CCR -= 5;
		}
      LL_TIM_OC_SetCompareCH1(TIM4, CCR);   
      LL_mDelay(50);
	}
}

void TIM_Base_Config(void){
	LL_TIM_InitTypeDef timbase_initstructure;	
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
	timbase_initstructure.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	timbase_initstructure.CounterMode = LL_TIM_COUNTERMODE_UP;
	timbase_initstructure.Autoreload = TIMx_ARR - 1;
	timbase_initstructure.Prescaler = TIMx_PSC - 1;
	LL_TIM_Init(TIM4 , &timbase_initstructure);
	LL_TIM_EnableCounter(TIM4);
}

void TIM_OC_GPIO_Config(void){
	LL_GPIO_InitTypeDef gpio_initstructure;	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	gpio_initstructure.Pin = SERVO_PIN;
	gpio_initstructure.Mode = LL_GPIO_MODE_ALTERNATE;
	gpio_initstructure.Alternate = LL_GPIO_AF_2;
	gpio_initstructure.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	gpio_initstructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	gpio_initstructure.Pull = LL_GPIO_PULL_UP;
	LL_GPIO_Init(GPIOB,&gpio_initstructure);
}

void TIM_OC_Config(void){
	LL_TIM_OC_InitTypeDef tim_oc_initstructure;	
	TIM_OC_GPIO_Config();
	TIM_Base_Config();
	tim_oc_initstructure.OCState = LL_TIM_OCSTATE_DISABLE;
	tim_oc_initstructure.OCMode = LL_TIM_OCMODE_PWM1;
	tim_oc_initstructure.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
	tim_oc_initstructure.CompareValue = CCR;
	LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH1, &tim_oc_initstructure);
	/*Start Output Compare in PWM Mode*/
	LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1);
	LL_TIM_EnableCounter(TIM4);
}

void ADC_Init (void){
// Enable ADC and GPIO clock
	RCC->APB2ENR |= (1<<9);  // enable ADC1 clock
	RCC->AHBENR |= (1<<0);  // enable GPIOA clock
	
// Set the Scan Mode and Resolution in the Control Register 1 (CR1)	
	//ADC1->CR1 = (1<<8);    // SCAN mode enabled
	ADC1->CR1 &= ~(3<<24);   // 12 bit RES
	
// Set the Continuous Conversion, EOC, and Data Alignment in Control Reg 2 (CR2)
	//ADC1->CR2 |= (1<<1);     // enable continuous conversion mode
	ADC1->CR2 |= (1<<10);    // EOC after each conversion
	ADC1->CR2 &= ~(1<<11);   // Data Alignment RIGHT
	
// Set the Sampling Time for the channels	
	ADC1->SMPR3 |= (2<<12);  // Sampling time of 3 cycles for channel 4 and channel 5
	ADC1->SMPR3 |= (2<<15);

// Set the Regular channel sequence length in ADC_SQR1
	ADC1->SQR1 |= (1<<20);   // SQR1_L =1 for 2 conversions
	
// Set the Respective GPIO PINs in the Analog Mode	
	GPIOA->MODER |= (3<<10);  // analog mode for PA 5 (chennel 5)
	GPIOA->MODER |= (3<<8);  // analog mode for PA 4 (channel 4)
}

void ADC_Enable (void){
	ADC1->CR2 |= 1<<0;   // ADON =1 enable ADC1
}

void ADC_Start (int channel){	
	
	ADC1->SQR5 = 0;
	ADC1->SQR5 |= (channel<<0);    // conversion in regular sequence
	
	ADC1->SR = 0;        // clear the status register
	
//	ADC1->CR2 |= (1<<30);  // start the conversion
}

void ADC_WaitForConv (void){
	do { ADC1->CR2 |= (1<<30); }
   while ((ADC1->SR & (1<<1))==0);  // wait for EOC flag to set
}

uint16_t ADC_GetVal (void){
	return ADC1->DR;  // Read the Data Register
}

void ADC_Disable (void){
	ADC1->CR2 &= ~(1<<0);  // Disable ADC
}

void SystemClock_Config(void)
{
  /* Enable ACC64 access and set FLASH latency */ 
  LL_FLASH_Enable64bitAccess();; 
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

  /* Set Voltage scale1 as MCU will run at 32MHz */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  
  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
  while (LL_PWR_IsActiveFlag_VOSF() != 0)
  {
  };
  
  /* Enable HSI if not already activated*/
  if (LL_RCC_HSI_IsReady() == 0)
  {
    /* HSI configuration and activation */
    LL_RCC_HSI_Enable();
    while(LL_RCC_HSI_IsReady() != 1)
    {
    };
  }
  
	
  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3);

  LL_RCC_PLL_Enable();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  };
  
  /* Sysclk activation on the main PLL */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  };
  
  /* Set APB1 & APB2 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  /* Set systick to 1ms in using frequency set to 32MHz                             */
  /* This frequency can be calculated through LL RCC macro                          */
  /* ex: __LL_RCC_CALC_PLLCLK_FREQ (HSI_VALUE, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3); */
  LL_Init1msTick(32000000);
  
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(32000000);
}
