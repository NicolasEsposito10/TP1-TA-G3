#ifndef DEVICE_H
#define DEVICE_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

class Device {
public:
    Device(uint8_t dhtPin, uint8_t dhtType, uint8_t potPin, uint8_t ledVent, uint8_t ledRiego, uint8_t buttonPin, Adafruit_SSD1306* display);
    void begin();
    void update();
    void handleButton();
    void showScreen();
    float getTemperature();
    float getHumidity();
    int getRefTemp();
    int getHumedadUmbral();
    bool isVentiladorActivo();
    bool isRiegoActivo();

private:
    DHT dht;
    Adafruit_SSD1306* display;
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