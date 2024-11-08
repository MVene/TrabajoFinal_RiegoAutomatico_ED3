# Proyecto Final - Sistema de Riego Automático

Este proyecto final fue desarrollado en la placa **LPC1769** y consiste en un sistema de riego automático que utiliza un **sensor capacitivo** para medir la humedad del suelo. La señal del sensor es procesada mediante un **conversor analógico-digital (ADC)**, y el sistema toma decisiones basadas en los valores obtenidos para activar o desactivar el riego de una planta.

## Tabla de Contenidos
1. [Descripción General](#descripción-general)
2. [Componentes del Sistema](#componentes-del-sistema)
3. [Funcionamiento](#funcionamiento)
4. [Configuración del Hardware](#configuración-del-hardware)
5. [Configuración del Software](#configuración-del-software)

## Descripción General

Este sistema de riego automático está diseñado para facilitar el mantenimiento de plantas mediante la automatización del proceso de riego. Utilizando un sensor capacitivo, el sistema detecta el nivel de humedad del suelo y, en función de ese valor, decide cuándo activar una bomba de agua para regar la planta. La **placa LPC1769** controla el proceso, convirtiendo las señales analógicas del sensor en valores digitales mediante el **ADC** y si este valor se encuentra entre 0 y 40%, la bomba se debe activar por cierto tiempo y luego apagarse. Ademas que estas mediciones se podran visualizar en la pc mediante una conexion **uart**.

## Componentes del Sistema

- **LPC1769**: Microcontrolador encargado del control general del sistema.
- **Sensor capacitivo de humedad**: Detecta la humedad en el suelo y envía una señal analógica al ADC.
- **Conversor ADC**: Convierte las señales analógicas del sensor en valores digitales.
- **Bomba de agua**: Actúa como el sistema de riego, controlado por el microcontrolador (por mientras led azul)
-**Led Rojo**: Nos permite saber si el sistema esta convirtiendo.

## Funcionamiento

1. **Lectura del Sensor**: El sensor capacitivo mide la humedad en el suelo y genera una señal analógica proporcional a la humedad.
2. **Conversión Analógica-Digital**: La señal analógica se convierte a un valor digital usando el ADC del microcontrolador LPC1769.
3. **Toma de Decisiones**: Basado en el valor digital, el microcontrolador decide si la humedad está por debajo del umbral(40%) configurado para iniciar el riego.
4. **Activación del Riego**: Si el nivel de humedad es bajo, el microcontrolador activa la bomba de agua (led azul).
5. **Monitoreo y Ajuste**: El sistema monitorea continuamente la humedad y ajusta el riego según sea necesario.
6. **Vizualizacion**: a definir
   
## Configuración del Hardware

1. **Conexión del Sensor Capacitivo**:
   - Conectar la salida del sensor al pin ADC del LPC1769 (ej. `ADC_CHANNEL_0`).
   - Asegurarse de que la señal analógica está dentro del rango del ADC (generalmente de 0V a 3.3V).
2. **Coneccion de leds**:
   - Led Rojo: nos indica que el sistema esta convirtiendo.
   - Led azul: nos indica que se esta regando.
     
## Configuracion del Software
