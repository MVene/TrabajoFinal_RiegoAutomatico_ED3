/*            	TRABAJO PRACTICO FINAL 
 *               ELECTRONICA DIGITAL 3
 *                        2024
 * 	
 * 		           "Riego Automático"
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
#include <stddef.h>

//-------- FUNCIONES --------//
void config_GPIO(void);
void config_ADC(void);
void config_TIMER0(void);
void config_TIMER1(void);

//-------- PROGRAMA PRINCIPAL --------//
void main(void){
    config_GPIO();
    config_ADC();
    config_TIMER0(); // timer 
    config_TIMER1(); // tiempo que dura el riego
    while(1){};
}
