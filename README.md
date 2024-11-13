# Proyecto Final - Sistema de Riego Automático

Este proyecto final fue desarrollado en la placa **LPC1769** y consiste en un sistema de riego automático que utiliza un **sensor capacitivo** para medir la humedad del suelo. La señal del sensor es procesada mediante un **conversor analógico-digital (ADC)**, y el sistema toma decisiones basadas en los valores obtenidos para activar o desactivar el riego de una planta.

## Tabla de Contenidos
1. [Componentes del Sistema](#componentes-del-sistema)
2. [Funcionamiento](#funcionamiento)




## Componentes del Sistema

- **LPC1769**: Microcontrolador encargado del control general del sistema.
- **Sensor capacitivo de humedad**: Detecta la humedad en el suelo y envía una señal analógica al ADC.
- **Bomba de agua**: Actúa como el sistema de riego, controlado por el microcontrolador (por mientras led azul).
- **Modulo UART**: Este comunicador serie nos permite observar el estado del sistema en la pc.
- **Led Azul**: Nos permite saber cuando la bomba se tiene que prender.
- **Led Verde**: Este led nos indica que la planta no necesita riego.
- **Led Naranja**: Nos indica que el sensor midio fuera de los parámetros.

## Funcionamiento

1. **Lectura del Sensor**: El sensor capacitivo mide la humedad en el suelo y genera una señal analógica proporcional a la humedad.
2. **Conversión Analógica-Digital**: La señal analógica se convierte a un valor digital usando el ADC del microcontrolador LPC1769.
3. **Toma de Decisiones**: Basado en el valor digital, el microcontrolador decide si la humedad está por debajo del umbral(40%) configurado para iniciar el riego.
4. **Activación del Riego**: Si el nivel de humedad es bajo (<40%), el microcontrolador activa la bomba de agua que esta conectada a un relé y ademas enciende el led azul.
5. **Monitoreo y Ajuste**: El sistema monitorea continuamente la humedad y ajusta el riego según sea necesario.
6. **Vizualizacion**: Mediante el modulo UART podemos ver el estado del sistema en la PC mediante un script de Python.
        

