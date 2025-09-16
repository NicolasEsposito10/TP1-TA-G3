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
    handleEncoder(); // Lee el estado del encoder y cambia la pantalla si se gira
    temp = dht.readTemperature(); // Lee la temperatura actual del sensor 
    hum = dht.readHumidity(); // Lee la humedad
    refTemp = map(analogRead(potPin), 0, 4095, 15, 35); // Lee el valor del potenciómetro y lo convierte a un rango de temperatura de referencia (15 a 35°C).

    // si esta seleccionada la pantalla de ventilacion la controla, en su defecto controla la de riego
    if (pantalla == 0)  
        controlVentilacion();
    else
        controlRiego();
    showScreen();
}

void Device::handleEncoder()
{
    int A = digitalRead(encA);
    if (A != lastA && A == LOW) // Nos fijamos si el encoder cambió de estado respecto al anterior
    {
        if (digitalRead(encB) == LOW)
        {
            pantalla = (pantalla + 1) % 2; // Giro derecha si B está en LOW
        }
        else
        {
            pantalla = (pantalla - 1 + 2) % 2; // Giro izquierda si B está en HIGH
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
        digitalWrite(ledVerde, HIGH); // Prende el led
    }
    else
    {
        if (ventiladorActivo)
            Serial.println("Ventilacion DESACTIVADA");
        ventiladorActivo = false;
        digitalWrite(ledVerde, LOW); //Apaga el led
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
            ultimoParpadeo = millis(); // tiempo actual
            estadoLedRiego = !estadoLedRiego; // Invierte el estado para que se encienda y se apague
            digitalWrite(ledVerde, estadoLedRiego); // Cambia el estado del led
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
