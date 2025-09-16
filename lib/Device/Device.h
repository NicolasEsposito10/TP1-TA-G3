#ifndef DEVICE_H
#define DEVICE_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

class Device
{
public:
    Device(uint8_t dhtPin, uint8_t dhtType, uint8_t potPin,
           uint8_t ledVerde, uint8_t encA, uint8_t encB, uint8_t encBtn,
           Adafruit_SSD1306 *display);

    void begin();
    void update();
    void showScreen();

    float getTemperature();
    float getHumidity();
    int getRefTemp();
    int getHumedadUmbral();
    bool isVentiladorActivo();
    bool isRiegoActivo();

private:
    DHT dht; // Declara un objeto de la clase DHT 
    Adafruit_SSD1306 *display; // Declara un puntero a un objeto de la clase Adafruit_SSD1306, que controla la pantalla OLED.
    uint8_t potPin, ledVerde;
    uint8_t encA, encB, encBtn;

    int pantalla;
    int humedadUmbral;
    bool ventiladorActivo;
    bool riegoActivo;
    unsigned long ultimoParpadeo;
    bool estadoLedRiego;
    float temp, hum;
    int refTemp;

    int lastA;

    // Metodos que solo púede usar la propia clase
    void controlVentilacion();
    void controlRiego();
    void handleEncoder();
};

#endif
