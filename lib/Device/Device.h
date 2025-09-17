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
    
    // Variables para el menú opcional
    bool modoMenu;
    int opcionMenu;
    bool lastBtn;
    unsigned long ultimoPresionBtn;
    static const int DEBOUNCE_DELAY = 200;
    static const int TOTAL_OPCIONES_MENU = 4;
    
    // Variables para control manual de valores
    bool tempManual;
    bool humedadManual;
    int tempRefManual;
    int humedadUmbralManual;
    
    // Variables para control manual de sistemas
    bool ventilacionManual;
    bool riegoManual;
    bool ventilacionForzado;
    bool riegoForzado;
    
    // Variables para navegación en control manual
    int sistemaSeleccionado; // 0=ventilacion, 1=riego
    int modoVentilacion; // 0=AUTO, 1=ON, 2=OFF
    int modoRiego; // 0=AUTO, 1=ON, 2=OFF

    // Metodos que solo púede usar la propia clase
    void controlVentilacion();
    void controlRiego();
    void handleEncoder();
    void handleMenu();
    void showMenu();
    void showEstadoCompleto();
    void modificarValores();
    void controlManual();
    void showControlManual();
};

#endif
