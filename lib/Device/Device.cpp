// Importa la cabecera de la clase Device para así poder utilizar sus definiciones
#include "Device.h"

// Constructor de la clase device, inicializa pines, leds, sensor y pantalla
Device::Device(uint8_t dhtPin, uint8_t dhtType, uint8_t potPin, uint8_t ledVent, uint8_t ledRiego, uint8_t buttonPin, Adafruit_SSD1306 *display)
    : dht(dhtPin, dhtType), display(display), potPin(potPin), ledVent(ledVent), ledRiego(ledRiego), buttonPin(buttonPin)
{
    pantalla = 0;
    ventiladorActivo = false;
    riegoActivo = false;
    ultimoParpadeo = 0;
    estadoLedRiego = false;
}

void Device::begin()
{
    dht.begin();
    // Se define el pin como output ya que un led espera recibir señales para funcionar
    pinMode(ledVent, OUTPUT);
    pinMode(ledRiego, OUTPUT);
    // Se define el pin como input ya que espera recibir señales (pulsación del botón) para mandar un mensaje.
    // PULLUP signigica que el pin estará en HIGH (levantado) hasta que se conecte a GND (pulsación del botón)
    pinMode(buttonPin, INPUT_PULLUP);

    // De esta manera, el valor aleatorio quedaba seteado siempre en 46
    // randomSeed(analogRead(0));
    // humedadUmbral = random(40, 61);
    humedadUmbral = 40 + esp_random() % 21; // Valor verdaderamente aleatorio entre 40 y 60

    Serial.print("Sistema iniciado. Umbral de humedad: ");
    Serial.print(humedadUmbral);
    Serial.println("%");

    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println("Invernadero iniciado");
    display->print("Umbral humedad: ");
    display->print(humedadUmbral);
    display->println("%");
    display->display();
    delay(2000);
}

void Device::update()
{
    handleButton();
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    refTemp = map(analogRead(potPin), 0, 4095, 15, 35);
    controlVentilacion();
    controlRiego();
    showScreen();
}

void Device::handleButton()
{
    if (digitalRead(buttonPin) == LOW)
    {
        delay(200);
        pantalla = (pantalla + 1) % 2;
    }
}

void Device::controlVentilacion()
{
    if (temp > refTemp)
    {
        if (!ventiladorActivo)
            Serial.println("Ventilacion ACTIVADA");
        ventiladorActivo = true;
        digitalWrite(ledVent, HIGH);
    }
    else
    {
        if (ventiladorActivo)
            Serial.println("Ventilacion DESACTIVADA");
        ventiladorActivo = false;
        digitalWrite(ledVent, LOW);
    }
}

void Device::controlRiego()
{
    if (hum < humedadUmbral)
    {
        if (!riegoActivo)
            Serial.println("Riego ACTIVADO");
        riegoActivo = true;
        if (millis() - ultimoParpadeo > 500)
        {
            ultimoParpadeo = millis();
            estadoLedRiego = !estadoLedRiego;
            digitalWrite(ledRiego, estadoLedRiego);
        }
    }
    else
    {
        if (riegoActivo)
            Serial.println("Riego DESACTIVADO");
        riegoActivo = false;
        digitalWrite(ledRiego, LOW);
    }
}

void Device::showScreen()
{
    display->clearDisplay();
    display->setCursor(0, 0);
    if (pantalla == 0)
    {
        display->println("=== TEMPERATURA ===");
        display->print("Actual: ");
        display->print(temp);
        display->println(" C");
        display->print("Referencia: ");
        display->print(refTemp);
        display->println(" C");
        display->print("Ventilacion: ");
        display->println(ventiladorActivo ? "ON" : "OFF");
    }
    else
    {
        display->println("=== HUMEDAD ===");
        display->print("Actual: ");
        display->print(hum);
        display->println(" %");
        display->print("Umbral: ");
        display->print(humedadUmbral);
        display->println(" %");
        display->print("Riego: ");
        display->println(riegoActivo ? "ON" : "OFF");
    }
    display->display();
}

float Device::getTemperature() { return temp; }
float Device::getHumidity() { return hum; }
int Device::getRefTemp() { return refTemp; }
int Device::getHumedadUmbral() { return humedadUmbral; }
bool Device::isVentiladorActivo() { return ventiladorActivo; }
bool Device::isRiegoActivo() { return riegoActivo; }