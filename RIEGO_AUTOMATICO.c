/*            	TRABAJO PRACTICO FINAL 
 *               ELECTRONICA DIGITAL 3
 *                        2024
 * 	
 * 		           "Sistema de Riego Automático"
 *	-----------------------------------------------------
 *	Integrantes:
                - Guimpelevich María Lujan
                - Venecia Milagros Ailín
 */


//-------- HEADERS --------//
#include "LPC17xx.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_UART.h"
//#include <stddef.h>

//-------- VARIABLES --------//

#define muestras_size 2000 //cantidad de muestras
volatile uint16_t muestras[muestras_size]; //buffer de muestras

volatile contador_muestras = 2000; // se va a restar uno cada vez que se complete una muestra

volatile promedio = 0;



//char resultadoHum[]	=	" ";					// Para enviar por UART


//uint8_t RegHumDMA[1] = {0};						    // Para enviar por UART usando DMA

//-------- FUNCIONES --------//
void config_GPIO(void);
void config_ADC(void);
void config_TIMER0(void);
void config_TIMER1(void);
//void config_DMA(void);
//void config_UART(void);
void visualizar_DMA_UART();

//-------- PROGRAMA PRINCIPAL --------//
int main(void){
    GPIO_ClearValue(0, 1<<0); //Apago Led azul pin 0.1
    GPIO_ClearValue(0, 1<<1); //Apago Led azul pin 0.1
    GPIO_ClearValue(0, 1<<2); //Apago Led azul pin 0.1
    GPIO_ClearValue(0, 1<<3); //Apago Led azul pin 0.1
    GPIO_ClearValue(0, 1<<23); //Apago Led azul pin 0.1
    
    config_GPIO();
    config_ADC();
    config_TIMER0(); // timer que cuenta cada cuanto tomo el promedio
    config_TIMER1(); // tiempo que dura el riego
  

    while(1){}

    return 0;   
}

// Configuracion de los pines
void config_GPIO(){
// 0.23 fun 1 adc - uart como fede - pines 0.0 , 0.1 y 0.2 para leds

    //Configuracion para pin 0.0 - led Rojo (ADC convirtiendo)
    PINSEL_CFG_Type pinselConfig;
    pinselConfig.Portnum = PINSEL_PORT_0; //puerto 0
    pinselConfig.Pinnum = PINSEL_PIN_0; // pin 0
    pinselConfig.Funcnum = PINSEL_FUNC_0; //modo GPIO
    pinselConfig.Pinmode = PINSEL_PINMODE_TRISTATE; //Sin Resistencias
    pinselConfig.OpenDrain = PINSEL_PINMODE_NORMAL;

    PINSEL_ConfigPin(&pinselConfig);

    //Configuracion para pin 0.1 - led azul (Bomba encendida)
    pinselConfig.Pinnum = PINSEL_PIN_1; // pin 1

    PINSEL_ConfigPin(&pinselConfig);

    //Configuracion para pin 0.2 - led verde (No necesita riego)
    pinselConfig.Pinnum = PINSEL_PIN_2; // pin 2

    PINSEL_ConfigPin(&pinselConfig);

    //Configuracion para pin 0.3 - led naranja (Alarma)
    pinselConfig.Pinnum = PINSEL_PIN_3; // pin 3

    PINSEL_ConfigPin(&pinselConfig);
    
    //Configuracion para pin 0.23 - channel 0 ADC
    pinselConfig.Pinnum = PINSEL_PIN_23; // pin 23
    pinselConfig.Funcnum = PINSEL_FUNC_1; //modo adc

    PINSEL_ConfigPin(&pinselConfig);

    /*//Configuracion para pin - Uart tx

    // TXD2
    pinselConfig.Portnum = 0; //port 0
    pinselConfig.Pinnum = 10; //pin 10
    pinselConfig.Funcnum = 1; //funcion tx/rx (igual para los 2 pines)
    
	
	PINSEL_ConfigPin(&pinselConfig); */

    GPIO_SetDir(0, 1<<0, 1); //pin 0.0 como salida
	GPIO_SetDir(0, 1<<1, 1); //pin 0.1 como salida
    GPIO_SetDir(0, 1<<2, 1); //pin 0.2 como salida
	GPIO_SetDir(0, 1<<3, 1); //pin 0.3 como salida

}



// Configuracion del ADC
void config_ADC(void){
    
    ADC_Init(LPC_ADC, 20000);    // inicializa el ADC a 200kHz de frecuencia de muestreo
    ADC_ChannelCmd(LPC_ADC, 0, ENABLE); // habilita el canal 0
    ADC_BurstCmd(LPC_ADC, DISABLE); // deshabilita el modo burst
    
    ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE); // habilita la interrupcion del canal 
    //NVIC_EnableIRQ(ADC_IRQn); // habilita la interrupcion del ADC
    NVIC_SetPriority(ADC_IRQn, 2);
    ADC_GlobalGetStatus(LPC_ADC, 1);					// Limpia la bandera del ADC

    ADC_StartCmd(LPC_ADC, ADC_START_NOW); //comienza la conversion
    GPIO_SetValue(0, 1<<0); //Enciendo Led rojo

}

//Configuracion del timer 0 para que interrumpa cada 5 minutos
void config_TIMER0(void){
    
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
    matchConfig.MatchValue = 300000000 - 1;  // 5 minutos
    
    TIM_ConfigMatch(LPC_TIM0, &matchConfig);
    
    TIM_ResetCounter(LPC_TIM0);
    TIM_Cmd(LPC_TIM0, ENABLE); //inicia el timer

    NVIC_SetPriority(TIMER0_IRQn, 1); 
    NVIC_EnableIRQ(TIMER0_IRQn);    
    TIM_ClearIntPending(LPC_TIM0,TIM_MR0_INT); //Limpio bandera de interrupcion del timer0

}

//Configuracion TIMER 1 para que se encienda la bomba durante 5 segundos
void config_TIMER1(){

    //nota: 
    TIM_TIMERCFG_Type timerConfig;

    // Configuracion del prescaler para que el timer cuente en microsegundos
    timerConfig.PrescaleOption = TIM_PRESCALE_TICKVAL; //milisegundos
    timerConfig.PrescaleValue = 25 - 1; // Divide el reloj de 25 MHz para contar en microsegundos

    // Iniciar el Timer1 en modo Timer
    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &timerConfig);

    // Configuracion del canal de coincidencia (Match) para generar una interrupcion cada 5 segundos
    TIM_MATCHCFG_Type matchConfig;

    matchConfig.MatchChannel = 0;
    matchConfig.IntOnMatch = ENABLE;
    matchConfig.ResetOnMatch = ENABLE;      // Resetea el Timer al llegar al valor de coincidencia
    matchConfig.StopOnMatch = ENABLE;      // No detiene el Timer
    matchConfig.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    matchConfig.MatchValue = 5000000 - 1;       // 5 segundos (5,000,000 us)

    // Configurar el canal de coincidencia en Timer1
    TIM_ConfigMatch(LPC_TIM1, &matchConfig);

    // Habilitar interrupcion para Timer1
    NVIC_EnableIRQ(TIMER1_IRQn);
    NVIC_SetPriority(TIMER0_IRQn, 3);
    TIM_ClearIntPending(LPC_TIM1,TIM_MR1_INT); //Limpio bandera de interrupcion del timer0


}

//Configuro DMA para que transporte el promedio hasta UART
void config_DMA(){

	GPDMA_Init();                // Inicializo el controlador de DMA

	GPDMA_Channel_CFG_Type DMAUARTConfig;
	DMAUARTConfig.ChannelNum = 0;
	DMAUARTConfig.SrcMemAddr = promedio;
	DMAUARTConfig.DstMemAddr = 0;
	DMAUARTConfig.TransferSize = sizeof(promedio);
	DMAUARTConfig.TransferWidth = 0;
	DMAUARTConfig.TransferType = GPDMA_TRANSFERTYPE_M2P;
	DMAUARTConfig.SrcConn = 0;
	DMAUARTConfig.DstConn = GPDMA_CONN_UART2_Tx;
	DMAUARTConfig.DMALLI = 0;

	GPDMA_Setup(&DMAUARTConfig);
    GPDMA_ChannelCmd(0,ENABLE);
	return;
}

//configuro UART
void config_UART(){
	UART_CFG_Type UARTConfigStruct;									// Variable auxiliar
																	// Es una estructura utilizada por las funciones del driver
																	// para mostrar/sacar valores y realizar la configuracion
	UART_ConfigStructInit(&UARTConfigStruct);

	UART_Init(LPC_UART2, &UARTConfigStruct);						// Inicializa periferico

	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	UARTFIFOConfigStruct.FIFO_DMAMode		=	ENABLE;
	UARTFIFOConfigStruct.FIFO_Level			=	UART_FIFO_TRGLEV0;
	UARTFIFOConfigStruct.FIFO_ResetRxBuf	=	ENABLE;
	UARTFIFOConfigStruct.FIFO_ResetTxBuf	=	ENABLE;
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	UART_FIFOConfig(LPC_UART2, &UARTFIFOConfigStruct);				// Inicializa FIFO

	UART_TxCmd(LPC_UART2, ENABLE);									// Habilita transmision

	UART_IntConfig(LPC_UART2, UART_INTCFG_RBR, ENABLE);				// Habilita interrupcion por el RX del UART
	UART_IntConfig(LPC_UART2, UART_INTCFG_RLS, ENABLE);				// Habilita interrupcion por el estado de la linea UART

	NVIC_EnableIRQ(UART2_IRQn);										// Habilita las interrupciones por UART2

	return;
}
void visualizar_DMA_UART(){
    config_DMA();
    config_UART();
}






//-------- HANDLERS --------//

void TIMER0_IRQHandler(){
   
    ADC_StartCmd(LPC_ADC, ADC_START_NOW); // El ADC comienza a convertir
    GPIO_SetValue(0, 1<<0); //Enciendo Led rojo
    TIM_ClearIntPending(LPC_TIM0,TIM_MR0_INT); //Limpio bandera de interrupcion del timer0
}

void TIMER1_IRQHandler(){
   
    GPIO_ClearValue(0, 1<<1); //Apago Led azul pin 0.1
    TIM_ClearIntPending(LPC_TIM1,TIM_MR0_INT); //Limpio bandera de interrupcion del timer1

}

void ADC_IRQHandler(){

    uint16_t valor = ADC_GlobalGetData(LPC_ADC);
    muestras[contador_muestras] = valor; 

    contador_muestras--; //cantidad de muestras restantes


    if(contador_muestras < 0){
        GPIO_ClearValue(0, 1<<0); // Apago Led rojo pin 0

        uint16_t suma = 0; // suma de todas las muestras

        //sumo las muestras
        for(int i=0; i<muestras_size; i++){
            suma+=muestras[i];
            muestras[i]=0; // voy limpiando el buffer miestras extraigo las muestras
        }
        //tomo el promedio
        promedio = suma/muestras_size;
        suma = 0; // reseteo la suma

        
        //visualizar_DMA_UART(); //Se configuran DMA y UART para que se muestre la humedad por pantalla


        if(promedio <= 3138 && promedio >= 1701){
            GPIO_SetValue(0, 1<<1); //Enciendo Led azul pin 0.1 (prendo bomba)
            TIM_Cmd(LPC_TIM1, ENABLE); //Inicia el Timer1, cuenta 5segundos e interrumpe
        }else if(promedio > 3138 && promedio <= 4096){
            GPIO_SetValue(0, 1<<2); // Enciendo Led verde pin 0.2 (no necesita riego)
        }else{
            GPIO_SetValue(0, 1<<3); // Enciendo Led naranja pin 0.3 (no deberia pasar esta situacion)
            //Mensaje de ERROR! en uart
        }

        promedio = 0; //limpio la variable
        ADC_GlobalGetStatus(LPC_ADC, 1);					// Limpia la bandera del ADC
        //flag abajo 
        return;
    }

    
    ADC_GlobalGetStatus(LPC_ADC, 1);					// Limpia la bandera del ADC
    ADC_StartCmd(LPC_ADC, ADC_START_NOW); //comienza la conversion nuevamente
}








/*	Funcion que convierte los valores de integers a ASCII
 *
 * 	@param	num:	integer a convertir
 * 	@param	str:	destino del integer convertido a ASCII
 * 	@return ninguno
 */
void itoa(uint16_t num, char* str){
    int i	=	0;

    // Manejo del numero 0 de forma explicita
    if (num == 0){
        str[i++]	=	'0';
        str[i++]	=	'\0';
        str[i] 		=	'\0';

        return;
    }

    // Procesa cada digito de manera individual
    while (num != 0){
        int rem		=	num % 10;
        str[i++]	=	(rem > 9)? (rem-10) + 'a' : rem + '0';   // Devuelve el equivalente en ASCII en orden inverso
        num = num/10;
    }

    if(i==1){
    	str[i++] = '\0';
    }

    str[i] = '\0';

    int start = 0;
    int end = i -1;
    char aux;

    // Invierte los caracteres del string
	while (start < end){
		aux				=	*(str+start);
		*(str+start)	=	*(str+end);
		*(str+end)		=	aux;
		start++;
		end--;
	}

    return;
}