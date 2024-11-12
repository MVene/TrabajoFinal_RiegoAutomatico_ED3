/*FUNCIONA!!*/
#include "lpc17xx_uart.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_pinsel.h"

// Buffer con el mensaje a enviar
char mensaje[] = "Hola, PC usando UART con DMA!\n";

void configPIN(){
    //Configuracion para pin - Uart tx

	PINSEL_CFG_Type PinCfg;
	
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	
	PINSEL_ConfigPin(&PinCfg);
}

// Configuración de UART
void config_UART() {
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
    //LPC_SC->PCONP |= (1 << 29);  // Habilitar GPDMA (bit 29 en PCONP)
    
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

    // Configura y habilita el canal de DMA
    GPDMA_Setup(&DMAUARTConfig);
    GPDMA_ChannelCmd(0, ENABLE); // Activa el canal DMA 0
}

int main() {
    configPIN();
    config_UART(); // Configura la UART para DMA
    config_DMA(); // Configura el DMA para enviar el mensaje

    while (1) {
        // Puedes agregar otras tareas aquí mientras el DMA envía el mensaje
    }

    return 0;
}
