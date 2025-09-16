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
    DHT dht;
    Adafruit_SSD1306 *display;
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

    void controlVentilacion();
    void controlRiego();
    void handleEncoder();
};

#endif
