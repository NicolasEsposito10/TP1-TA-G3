#include "Device.h"

Device::Device(uint8_t dhtPin, uint8_t dhtType, uint8_t potPin,
               uint8_t ledVerde, uint8_t encA, uint8_t encB, uint8_t encBtn,
               Adafruit_SSD1306 *display)
    : dht(dhtPin, dhtType), display(display),
      potPin(potPin), ledVerde(ledVerde),
      encA(encA), encB(encB), encBtn(encBtn)
{
    pantalla = 0;
    ventiladorActivo = false;
    riegoActivo = false;
    ultimoParpadeo = 0;
    estadoLedRiego = false;
    lastA = HIGH;
}

void Device::begin()
{
    dht.begin();
    pinMode(ledVerde, OUTPUT);

    pinMode(encA, INPUT_PULLUP);
    pinMode(encB, INPUT_PULLUP);
    pinMode(encBtn, INPUT_PULLUP);

    humedadUmbral = 40 + esp_random() % 21;

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
    handleEncoder();
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    refTemp = map(analogRead(potPin), 0, 4095, 15, 35);
    if (pantalla == 0)
        controlVentilacion();
    else
        controlRiego();
    showScreen();
}

void Device::handleEncoder()
{
    int A = digitalRead(encA);
    if (A != lastA && A == LOW)
    {
        if (digitalRead(encB) == LOW)
        {
            pantalla = (pantalla + 1) % 2; // Giro derecha
        }
        else
        {
            pantalla = (pantalla - 1 + 2) % 2; // Giro izquierda
        }
        Serial.print("Cambio a Pantalla: ");
        Serial.println(pantalla == 0 ? "TEMPERATURA" : "HUMEDAD");
    }
    lastA = A;
}

void Device::controlVentilacion()
{
    if (temp > refTemp)
    {
        if (!ventiladorActivo)
            Serial.println("Ventilacion ACTIVADA");
        ventiladorActivo = true;
        digitalWrite(ledVerde, HIGH);
    }
    else
    {
        if (ventiladorActivo)
            Serial.println("Ventilacion DESACTIVADA");
        ventiladorActivo = false;
        digitalWrite(ledVerde, LOW);
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
            digitalWrite(ledVerde, estadoLedRiego);
        }
    }
    else
    {
        if (riegoActivo)
            Serial.println("Riego DESACTIVADO");
        riegoActivo = false;
        digitalWrite(ledVerde, LOW);
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

// Getters
float Device::getTemperature() { return temp; }
float Device::getHumidity() { return hum; }
int Device::getRefTemp() { return refTemp; }
int Device::getHumedadUmbral() { return humedadUmbral; }
bool Device::isVentiladorActivo() { return ventiladorActivo; }
bool Device::isRiegoActivo() { return riegoActivo; }
