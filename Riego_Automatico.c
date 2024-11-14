/*            	TRABAJO PRACTICO FINAL 
 *               ELECTRONICA DIGITAL 3
 *                        2024
 * 	
 * 		      "Sistema de Riego Automático"
 *	-----------------------------------------------------
 *	Integrantes:
                - Guimpelevich María Luján
                - Venecia Milagros Ailín
 */

//-------- HEADERS --------//

#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_UART.h"
#include "string.h"
#include "stdio.h"

//-------- VARIABLES --------//

// PINES LEDs
#define BOMBA  (1<<0)  // P0.0
#define LED_AZUL  (1<<1)  // P0.1
#define LED_VERDE  (1<<2)  // P0.2
#define LED_NARANJA  (1<<3)  // P0.3

// Umbral de HUMEDAD 
#define HUMEDAD_MINIMA 3000
#define UMBRAL_HUMEDAD 2373 
#define HUMEDAD_MAXIMA 1300
 
// Buffer con el mensaje a enviar
char mensaje[32] = " "; 
volatile uint32_t adc_value = 0;

//-------- FUNCIONES --------//

//Configuracion ADC
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
    matchConfig.MatchValue = 20000000;  // 20 segundos
    
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

//Configuracion pines
void config_pin(void) {

    // GPIO por defecto
    // 0.0 - Bomba
	// P0.1 - led AZUL
    // P0.2 - led VERDE
	// P0.3 - led NARANJA
    
    GPIO_SetDir(0,  BOMBA | LED_AZUL | LED_VERDE | LED_NARANJA, 1);
    GPIO_ClearValue(0, LED_AZUL | LED_VERDE | LED_NARANJA);
    GPIO_SetValue(0, BOMBA);
}


// Configuración de UART
void config_UART() {

    //Configuracion para pin - Uart tx
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	
	PINSEL_ConfigPin(&PinCfg);

    UART_CFG_Type UARTConfigStruct;
    UART_ConfigStructInit(&UARTConfigStruct);
    UART_Init(LPC_UART2, &UARTConfigStruct);

    UART_FIFO_CFG_Type UARTFIFOConfigStruct;
    UARTFIFOConfigStruct.FIFO_DMAMode = ENABLE; // Habilita el modo DMA
    UARTFIFOConfigStruct.FIFO_Level = UART_FIFO_TRGLEV0;
    UARTFIFOConfigStruct.FIFO_ResetRxBuf = ENABLE;
    UARTFIFOConfigStruct.FIFO_ResetTxBuf = ENABLE;
    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

    UART_FIFOConfig(LPC_UART2, &UARTFIFOConfigStruct);
    UART_TxCmd(LPC_UART2, ENABLE);  // Habilita transmisión
}

// Configuración de DMA para enviar mensaje
void config_DMA() {
    
    GPDMA_Init(); // Inicializa el controlador de DMA

    // Configura el canal de DMA para transferir datos desde el buffer a UART
    GPDMA_Channel_CFG_Type DMAUARTConfig;
    DMAUARTConfig.ChannelNum = 0; // Canal 0 de DMA
    DMAUARTConfig.SrcMemAddr = (uint32_t)mensaje; // Dirección de inicio del mensaje
    DMAUARTConfig.DstMemAddr = 0; // Registro de transmisión de UART2
    DMAUARTConfig.TransferSize = sizeof(mensaje); // Tamaño del mensaje
    DMAUARTConfig.TransferWidth = 0; // Transfiere en bytes
    DMAUARTConfig.TransferType = GPDMA_TRANSFERTYPE_M2P; // Transferencia de Memoria a Periférico
    DMAUARTConfig.SrcConn = 0; // No se requiere conexión de fuente
    DMAUARTConfig.DstConn = GPDMA_CONN_UART2_Tx; // Conexión de destino UART2 Tx
    DMAUARTConfig.DMALLI = 0; // Sin enlace a otra transferencia

    // Configura 
    GPDMA_Setup(&DMAUARTConfig);
    GPDMA_ChannelCmd(0, ENABLE); // Activa el canal DMA 0
}

// Visualiza el mensaje en la UART
void visualizar_DMA_UART(){

    config_UART();
    config_DMA();
}

//-------- HANDLERS --------//

void TIMER0_IRQHandler(void) {

    if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT) == SET) {

        ADC_StartCmd(LPC_ADC, ADC_START_NOW);

        while (!(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE)));

        adc_value = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
        
        if(adc_value <= UMBRAL_HUMEDAD && adc_value >= HUMEDAD_MAXIMA){

            GPIO_SetValue(0, LED_VERDE); // Enciendo Led verde pin 0.2 (no necesita riego)
            TIM_Cmd(LPC_TIM1, ENABLE); //Inicia el Timer1, cuenta 5 segundos e interrumpe
            strcpy(mensaje, "Humedad alta: NO necesita riego \n");
            visualizar_DMA_UART();

        }else if(adc_value > UMBRAL_HUMEDAD && adc_value <= HUMEDAD_MINIMA){

            GPIO_SetValue(0, LED_AZUL); //Enciendo Led azul pin 0.1 (prendo bomba)
            GPIO_ClearValue(0, BOMBA); //Mando cero logico para encender pin 0.0 (prendo bomba)
            TIM_Cmd(LPC_TIM1, ENABLE); //Inicia el Timer1, cuenta 5segundos e interrumpe
            strcpy(mensaje, "Humedad baja: iniciando riego \n");
            visualizar_DMA_UART();

        }else{

            GPIO_SetValue(0, LED_NARANJA); // Enciendo Led naranja pin 0.3 (no deberia pasar esta situacion)
            TIM_Cmd(LPC_TIM1, ENABLE); //Inicia el Timer1, cuenta 5segundos e interrumpe
            strcpy(mensaje, "¡Error! Valor fuera de limite \n");
            visualizar_DMA_UART();

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
        GPIO_SetValue(0, BOMBA); //Apago la bomba con cero en pin 0.
        TIM_ClearIntPending(LPC_TIM1,TIM_MR0_INT); //Limpio bandera de interrupcion del timer1

    }
}

//-------- PROGRAMA PRINCIPAL --------//

int main(void) {

    config_pin(); // Configura los pines
    init_adc(); // Configura el ADC
    config_timer0(); // Configura el timer0
    config_timer1(); // Configura el timer1
    
    // Bucle infinito
    while(1) { 
    }

    return 0;
}