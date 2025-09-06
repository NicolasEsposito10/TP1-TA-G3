#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- CONFIGURACIÓN HARDWARE ---
#define DHTPIN 15          // Pin del DHT22
#define DHTTYPE DHT22
#define POTPIN 34          // Potenciómetro
#define LED_VENT 2         // LED ventilación
#define LED_RIEGO 4        // LED riego
#define BUTTON 5           // Botón para cambiar pantallas

// --- OBJETOS ---
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire);

// --- VARIABLES ---
int pantalla = 0;                // Pantalla actual (0 = temp, 1 = humedad)
int humedadUmbral;               // Umbral aleatorio de humedad [40-60]
bool ventiladorActivo = false;   // Estado ventilación
bool riegoActivo = false;        // Estado riego
unsigned long ultimoParpadeo = 0;
bool estadoLedRiego = false;

void setup() {
  Serial.begin(115200);

  // Inicializar DHT
  dht.begin();

  // Inicializar OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Error al inicializar OLED");
    for (;;);
  }
  display.clearDisplay();
  display.display();

  // Pines
  pinMode(LED_VENT, OUTPUT);
  pinMode(LED_RIEGO, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  // Generar umbral aleatorio
  randomSeed(analogRead(0));
  humedadUmbral = random(40, 61);

  Serial.print("Sistema iniciado. Umbral de humedad: ");
  Serial.print(humedadUmbral);
  Serial.println("%");

  // Mostrar mensaje inicial
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Invernadero iniciado");
  display.print("Umbral humedad: ");
  display.print(humedadUmbral);
  display.println("%");
  display.display();

  delay(2000);
}

void loop() {
  // Leer botón (cambia de pantalla)
  if (digitalRead(BUTTON) == LOW) {
    delay(200); // antirrebote
    pantalla = (pantalla + 1) % 2;
  }

  // Leer sensores
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int refTemp = map(analogRead(POTPIN), 0, 4095, 15, 35); // referencia entre 15°C y 35°C

  // --- CONTROL VENTILACIÓN ---
  if (temp > refTemp) {
    if (!ventiladorActivo) {
      Serial.println("Ventilacion ACTIVADA");
    }
    ventiladorActivo = true;
    digitalWrite(LED_VENT, HIGH);
  } else {
    if (ventiladorActivo) {
      Serial.println("Ventilacion DESACTIVADA");
    }
    ventiladorActivo = false;
    digitalWrite(LED_VENT, LOW);
  }

  // --- CONTROL RIEGO ---
  if (hum < humedadUmbral) {
    if (!riegoActivo) {
      Serial.println("Riego ACTIVADO");
    }
    riegoActivo = true;

    // Parpadeo LED
    if (millis() - ultimoParpadeo > 500) {
      ultimoParpadeo = millis();
      estadoLedRiego = !estadoLedRiego;
      digitalWrite(LED_RIEGO, estadoLedRiego);
    }
  } else {
    if (riegoActivo) {
      Serial.println("Riego DESACTIVADO");
    }
    riegoActivo = false;
    digitalWrite(LED_RIEGO, LOW);
  }

  // --- MOSTRAR EN OLED ---
  display.clearDisplay();
  display.setCursor(0, 0);

  if (pantalla == 0) {
    // Pantalla Temperatura
    display.println("=== TEMPERATURA ===");
    display.print("Actual: ");
    display.print(temp);
    display.println(" C");

    display.print("Referencia: ");
    display.print(refTemp);
    display.println(" C");

    display.print("Ventilacion: ");
    display.println(ventiladorActivo ? "ON" : "OFF");
  } else {
    // Pantalla Humedad
    display.println("=== HUMEDAD ===");
    display.print("Actual: ");
    display.print(hum);
    display.println(" %");

    display.print("Umbral: ");
    display.print(humedadUmbral);
    display.println(" %");

    display.print("Riego: ");
    display.println(riegoActivo ? "ON" : "OFF");
  }

  display.display();

  delay(500);
}
