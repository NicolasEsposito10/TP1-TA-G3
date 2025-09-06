#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Device.h"

#define DHTPIN 15
#define DHTTYPE DHT22
#define POTPIN 34
#define LED_VENT 2
#define LED_RIEGO 4
#define BUTTON 5

Adafruit_SSD1306 display(128, 64, &Wire);
Device device(DHTPIN, DHTTYPE, POTPIN, LED_VENT, LED_RIEGO, BUTTON, &display);

void setup() {
    Serial.begin(115200);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Error al inicializar OLED");
        for (;;);
    }
    device.begin();
}

void loop() {
    device.update();
    delay(500);
}