#include <Arduino.h>
// #include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Device.h"

const int DHTPIN = GPIO_NUM_33;
// GPIO_NUM_33 = #define DHTPIN 33 // Pin donde está conectado el sensor DHT
#define DHTTYPE DHT22           // Redefine tipo de DHT a DHT22
const int POTPIN = GPIO_NUM_32; // Pin donde está conectado el potenciómetro
const int PIN_LED_V = GPIO_NUM_23;
#define BUTTON 5

Adafruit_SSD1306 display(128, 64, &Wire);
Device device(DHTPIN, DHTTYPE, POTPIN, PIN_LED_V, BUTTON, &display);

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ; // Espera a que se abra el monitor serie
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("Error al inicializar OLED");
        for (;;)
            ;
    }
    Serial.println("Sistema iniciado");
    device.begin();

    Serial.println("Sistema iniciado");
}

void loop()
{
    device.update();
    delay(200);
}