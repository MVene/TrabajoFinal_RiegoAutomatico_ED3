#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"

// PINES LEDs
#define BOMBA  (1<<0)  // P0.0
#define LED_AZUL  (1<<1)  // P0.1
#define LED_VERDE  (1<<2)  // P0.2
#define LED_NARANJA  (1<<3)  // P0.3

// Umbral de HUMEDAD 
#define HUMEDAD_MAXIMA 3000
#define UMBRAL_HUMEDAD 2373 
#define HUMEDAD_MINIMA 1300
 
volatile uint32_t adc_value = 0;


void init_adc(void) {
    // Configurar P0.23 como AD0.0
    PINSEL_CFG_Type pinsel_cfg;
    pinsel_cfg.Portnum = 0;
    pinsel_cfg.Pinnum = 23;
    pinsel_cfg.Funcnum = 1;
    pinsel_cfg.Pinmode = 0; 
    PINSEL_ConfigPin(&pinsel_cfg);

    // Inicializar ADC
    ADC_Init(LPC_ADC, 200000); // 200kHz ADC clock
    ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
    ADC_BurstCmd(LPC_ADC, DISABLE);
}

// Configuracion del Timer0
void config_timer0(void) {
    TIM_TIMERCFG_Type timerConfig;
    TIM_MATCHCFG_Type matchConfig;
    
    timerConfig.PrescaleOption = TIM_PRESCALE_TICKVAL;
    timerConfig.PrescaleValue = 24;  // 1 microsegundo
    
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timerConfig);
    
    matchConfig.MatchChannel = 0;
    matchConfig.IntOnMatch = ENABLE;
    matchConfig.ResetOnMatch = ENABLE;
    matchConfig.StopOnMatch = DISABLE;
    matchConfig.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    matchConfig.MatchValue = 120000000;  // 2 minutos
    
    TIM_ConfigMatch(LPC_TIM0, &matchConfig);
    
    TIM_ResetCounter(LPC_TIM0);
    TIM_Cmd(LPC_TIM0, ENABLE); //inicia el timer

    TIM_ClearIntPending(LPC_TIM0,TIM_MR0_INT); //Limpio bandera de interrupcion del timer0
    NVIC_SetPriority(TIMER0_IRQn, 1); 
    NVIC_EnableIRQ(TIMER0_IRQn);    
}
   
// Configuracion del timer1
void config_timer1(void) {
    TIM_TIMERCFG_Type timerConfig;
    TIM_MATCHCFG_Type matchConfig;
    
    timerConfig.PrescaleOption = TIM_PRESCALE_TICKVAL;
    timerConfig.PrescaleValue = 24;  // 1 microsegundo
    
    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &timerConfig);
    
    matchConfig.MatchChannel = 0;
    matchConfig.IntOnMatch = ENABLE;
    matchConfig.ResetOnMatch = ENABLE;
    matchConfig.StopOnMatch = ENABLE;
    matchConfig.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    matchConfig.MatchValue = 5000000;  // 5 segundos
    
    TIM_ConfigMatch(LPC_TIM1, &matchConfig);
    
    TIM_ResetCounter(LPC_TIM1);


    TIM_ClearIntPending(LPC_TIM1,TIM_MR0_INT); //Limpio bandera de interrupcion del timer0
    NVIC_SetPriority(TIMER1_IRQn, 2); 
    NVIC_EnableIRQ(TIMER1_IRQn);    
}

void config_leds(void) {
    // GPIO por defecto
    //0.0 - Bomba
	// P0.1 - led AZUL
    // P0.2 - led VERDE
	// P0.3 - led NARANJA
    

    GPIO_SetDir(0,  BOMBA | LED_AZUL | LED_VERDE | LED_NARANJA, 1);
    GPIO_ClearValue(0, BOMBA | LED_AZUL | LED_VERDE | LED_NARANJA);
}

void TIMER0_IRQHandler(void) {

    if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT) == SET) {

        ADC_StartCmd(LPC_ADC, ADC_START_NOW);
        while (!(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE)));

        adc_value = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
        
        if(adc_value <= UMBRAL_HUMEDAD && adc_value >= HUMEDAD_MINIMA){
            GPIO_SetValue(0, LED_VERDE); // Enciendo Led verde pin 0.2 (no necesita riego)
            TIM_Cmd(LPC_TIM1, ENABLE); //Inicia el Timer1, cuenta 5 segundos e interrumpe
        }else if(adc_value > UMBRAL_HUMEDAD && adc_value <= HUMEDAD_MAXIMA){
            GPIO_SetValue(0, LED_AZUL); //Enciendo Led azul pin 0.1 (prendo bomba)
            GPIO_SetValue(0, BOMBA); //Enciendo pin 0.0 (prendo bomba)
            TIM_Cmd(LPC_TIM1, ENABLE); //Inicia el Timer1, cuenta 5segundos e interrumpe
        }else{
            GPIO_SetValue(0, LED_NARANJA); // Enciendo Led naranja pin 0.3 (no deberia pasar esta situacion)
            TIM_Cmd(LPC_TIM1, ENABLE); //Inicia el Timer1, cuenta 5segundos e interrumpe
            //Mensaje de ERROR! en uart
        }
        TIM_Cmd(LPC_TIM0, ENABLE); //inicia el timer
        TIM_ClearIntPending(LPC_TIM0,TIM_MR0_INT); //Limpio bandera de interrupcion del timer0
    }
}

void TIMER1_IRQHandler(void) {
    if (TIM_GetIntStatus(LPC_TIM1, TIM_MR0_INT) == SET) {
        GPIO_ClearValue(0, LED_AZUL); //Apago Led azul pin 0.1
        GPIO_ClearValue(0, LED_VERDE); //Apago Led verde pin 0.2
        GPIO_ClearValue(0, LED_NARANJA); //Apago Led naranja pin 0.3
        GPIO_ClearValue(0, BOMBA); //Apago la bomba pin 0.
        TIM_ClearIntPending(LPC_TIM1,TIM_MR0_INT); //Limpio bandera de interrupcion del timer1
    }
}

int main(void) {
    config_leds();
    init_adc();
    config_timer0();
    config_timer1();
    
    while(1) {
    }
    return 0;
}