/*EJEMPLO DE ADC CON TIMER FUNCIONAL*/

#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"

// PINES LEDs
#define LED_ROJO (1<<0)  // P0.0
#define LED_AZUL  (1<<1)  // P0.1

// Umbral de temperatura para encender alarma
#define UMBRAL_TEMPERATURA 40 // °C

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
    //ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
    ADC_BurstCmd(LPC_ADC, DISABLE);
    ADC_GlobalGetStatus(LPC_ADC, 1);// Limpia la bandera del ADC
    // Habilitar interrupción del ADC
   // NVIC_SetPriority(ADC_IRQn, 2);
    //NVIC_EnableIRQ(ADC_IRQn);
}

void config_timer(void) {
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
    matchConfig.MatchValue = 1000;  // 5 minutos

    TIM_ConfigMatch(LPC_TIM0, &matchConfig);

    TIM_ResetCounter(LPC_TIM0);
    TIM_Cmd(LPC_TIM0, ENABLE); //inicia el timer

    TIM_ClearIntPending(LPC_TIM0,TIM_MR0_INT); //Limpio bandera de interrupcion del timer0
    NVIC_SetPriority(TIMER0_IRQn, 1);
    NVIC_EnableIRQ(TIMER0_IRQn);

}


void config_leds(void) {
    // GPIO por defecto
	// P0.0 - led verde
	// P0.1 - led rojo
    GPIO_SetDir(0, LED_ROJO | LED_AZUL, 1);
    GPIO_ClearValue(0, LED_ROJO | LED_AZUL);
}

/*// Manejador de interrupción del ADC
void ADC_IRQHandler(void) {
    adc_value = ADC_ChannelGetData(LPC_ADC, 0); // 0 - 3.3V -> 0 - 4095

    // Convertir el valor ADC a temperatura (ajustar según el sensor)
    float temperatura = (adc_value * 3.3 / 4095.0) * 100.0; //  Pasa 0 - 4095 -> 0°C a 100°C

    if (temperatura > UMBRAL_TEMPERATURA) {
        GPIO_SetValue(0, LED_AZUL);
        GPIO_ClearValue(0, LED_ROJO);
    } else {
        GPIO_SetValue(0, LED_ROJO);
        GPIO_ClearValue(0, LED_AZUL);
    }
}*/

void TIMER0_IRQHandler(void) {

    if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT) == SET) {
        ADC_StartCmd(LPC_ADC, ADC_START_NOW);
        while (!(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE)));

        adc_value = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);

                // Convertir el valor ADC a temperatura (ajustar según el sensor)
        float temperatura = (adc_value * 3.3 / 4095.0) * 100.0; //  Pasa 0 - 4095 -> 0°C a 100°C

        if (temperatura > UMBRAL_TEMPERATURA) {
            GPIO_SetValue(0, LED_AZUL);
            GPIO_ClearValue(0, LED_ROJO);
        } else {
            GPIO_SetValue(0, LED_ROJO);
            GPIO_ClearValue(0, LED_AZUL);
        }

        TIM_ClearIntPending(LPC_TIM0,TIM_MR0_INT); //Limpio bandera de interrupcion del timer0
    }
}

int main(void) {
    config_leds();
    init_adc();
    config_timer();



    while(1) {
    }
    return 0;
}
