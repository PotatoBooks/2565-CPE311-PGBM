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


/*already implemented */
void SystemClock_Config(void);
void TIM_Base_Config(void); //function prototype
void TIM_OC_GPIO_Config(void); //function prototype
void TIM_OC_Config(void); //function prototype
void ADC_Config(void);

int pos = 90; // initial position of the Horizontal movement controlling servo motor
int tolerance = 20; // allowable tolerance setting - so solar servo motor isn't constantly in motion
int val1 = 0;
int val2 = 0;

int main()
{
  SystemClock_Config();
	TIM_OC_Config();
	ADC_Config();
	while(1){
		LL_ADC_REG_StartConversionSWStart(ADC1);
		while(LL_ADC_IsActiveFlag_EOCS(ADC1) == RESET);
		val1 = LL_ADC_REG_ReadConversionData12(ADC1);
		val2 = LL_ADC_REG_ReadConversionData12(ADC1);
		
		if((abs(val1 - val2) >= tolerance) || (abs(val2 - val1) >= tolerance)) {
			if(val1 > val2)
				pos += 1;
			else if(val1 < val2)
				pos -= 1;
		}
			if(pos > 180)
				pos = 180;
			if(pos < 0)
				pos = 0;
      LL_TIM_OC_SetCompareCH1(TIM4, pos);

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
	tim_oc_initstructure.CompareValue = 1500;
	LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH1, &tim_oc_initstructure);

	/*Start Output Compare in PWM Mode*/
	LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1);
	LL_TIM_EnableCounter(TIM4);
}

void ADC_Config(void)
{
    LL_RCC_HSI_Enable();
    while(!LL_RCC_HSI_IsReady()); //wait until HSI READY rise flag

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

    LL_GPIO_SetPinMode(GPIOA, LDR1_PIN, LL_GPIO_MODE_ANALOG);
    LL_GPIO_SetPinMode(GPIOA, LDR2_PIN, LL_GPIO_MODE_ANALOG);

    LL_ADC_InitTypeDef ADC_InitStruct;
    LL_ADC_StructInit(&ADC_InitStruct);
    ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
    ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
    ADC_InitStruct.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
    LL_ADC_Init(ADC1, &ADC_InitStruct);

    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_4);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_4, LL_ADC_SAMPLINGTIME_16CYCLES);

    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_2, LL_ADC_CHANNEL_5);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_5, LL_ADC_SAMPLINGTIME_16CYCLES);

    LL_ADC_Enable(ADC1);
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
