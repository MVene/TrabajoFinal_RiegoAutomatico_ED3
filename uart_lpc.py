import serial

def leer_desde_uart():
    # Configura el puerto serial (ajustar el puerto y la velocidad de baudios según configuración)
    puerto = 'COM3'  # Cambiar esto según el puerto en el que esté conectado el CP2102
    baudrate = 9600  # Asegurarse de que coincida con la configuración de UART del LPC

    try:
        # Abre el puerto serial
        with serial.Serial(port=puerto, baudrate=baudrate, timeout=1) as ser:
            print("Conectado al puerto serial:", puerto)
            
            # Lee continuamente los datos del LPC
            while True:
                if ser.in_waiting > 0:  # Si hay datos disponibles
                    datos = ser.readline().decode('utf-8').strip()  # Lee una línea completa y elimina espacios en blanco
                    print("Recibido desde LPC:", datos)
    
    except serial.SerialException as e:
        print("Error al abrir el puerto serial:", e)

# Llama a la función para iniciar la lectura
leer_desde_uart()
