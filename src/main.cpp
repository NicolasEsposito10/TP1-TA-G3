#include <Arduino.h>
#include <Adafruit_SSD1306.h> // Libreria para el oled
#include "Device.h" // Nuestra libreria :D


const int DHTPIN = GPIO_NUM_33;
#define DHTTYPE DHT22 // Le dice al programa que el sensor de temperatura y humedad es un DHT22.
const int POTPIN = GPIO_NUM_32;
const int PIN_LED = GPIO_NUM_23;

// Pines del encoder
const int ENC_A = GPIO_NUM_18;
const int ENC_B = GPIO_NUM_5;
const int ENC_BTN = GPIO_NUM_19;

Adafruit_SSD1306 display(128, 64, &Wire); // Objeto para controlar el OLED
Device device(DHTPIN, DHTTYPE, POTPIN, PIN_LED, ENC_A, ENC_B, ENC_BTN, &display); // Objeto clase device con los pines y referencia al oled

void setup()
{
    Serial.begin(115200);
    while (!Serial) //espera a que el puerto serie esté listo
        ;
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) // Inicializa pantalla oled
    {
        Serial.println("Error al inicializar OLED");
        for (;;)
            ;
    }
    Serial.println("Sistema iniciado");
    device.begin();
}

void loop()
{
    device.update();
    delay(2000);
}
