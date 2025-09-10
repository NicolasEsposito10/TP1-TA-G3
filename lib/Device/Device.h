// Esto evita que el archivo se incluya mas de una vez en la compilación
#ifndef DEVICE_H
#define DEVICE_H

#include <Arduino.h> // Defniciones básicas de Arduino (pinMode, digitalWrite, etc.)
// #include <Adafruit_GFX.h> => No es necesario ya que Adafruit_SSD1306 ya lo incluye
#include <Adafruit_SSD1306.h> // Para manejar pantallas OLED del modelo SSD1306
#include <DHT.h>              // Se utiliza para manejar sensores de temperatura y humedad DHT

class Device
{
    // public: se utiliza para definir los métodos y atributos accesibles desde fuera de la clase
public: // Se utiliza uint8_t para pines y tipos ya que son valores pequeños y se busca ahorrar memoria (1byte; 0 - 255)
    Device(uint8_t dhtPin, uint8_t dhtType, uint8_t potPin, uint8_t ledVent, uint8_t ledRiego, uint8_t buttonPin, Adafruit_SSD1306 *display);
    void begin();              // Inicializa los componentes del dispositivo
    void update();             // Actualiza el estado del dispositivo en base a las entradas y valores de los sensores
    void handleButton();       // Maneja la pulsación del botón para cambiar la pantalla
    void showScreen();         // Muestra la pantalla actual en el OLED
    float getTemperature();    // Obtiene la temperatura actual del sensor DHT
    float getHumidity();       // Obtiene la humedad actual del sensor DHT
    int getRefTemp();          // Obtiene la temperatura de referencia seteada por el potenciómetro
    int getHumedadUmbral();    // Obtiene un valor aleatorio para el umbral de humedad
    bool isVentiladorActivo(); // Indica si el ventilador está activo (ON/OFF)
    bool isRiegoActivo();      // Indica si el sistema de riego está activo (ON/OFF)

    // private: se utiliza para definir los métodos y atributos accesibles solo desde dentro de la clase
private:
    DHT dht;                   // dhtPin y dhType  ya están incluidos en objeto dht
    Adafruit_SSD1306 *display; // Puntero al valor de memoria ocupado por la instancia de la pantalla OLED
                               // Evita duplicados innecesarios de la pantalla en memoria
    uint8_t potPin, ledVent, ledRiego, buttonPin;
    int pantalla;
    int humedadUmbral;
    bool ventiladorActivo;
    bool riegoActivo;
    unsigned long ultimoParpadeo;
    bool estadoLedRiego;
    float temp, hum;
    int refTemp;
    void controlVentilacion();
    void controlRiego();
};

#endif