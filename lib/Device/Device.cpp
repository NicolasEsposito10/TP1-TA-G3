#include "Device.h"

Device::Device(uint8_t dhtPin, uint8_t dhtType, uint8_t potPin,
               uint8_t ledVerde, uint8_t encA, uint8_t encB, uint8_t encBtn,
               Adafruit_SSD1306 *display)
    : dht(dhtPin, dhtType), display(display),
      potPin(potPin), ledVerde(ledVerde),
      encA(encA), encB(encB), encBtn(encBtn) //Constructor
{
    pantalla = 0;
    ventiladorActivo = false;
    riegoActivo = false;
    ultimoParpadeo = 0;
    estadoLedRiego = false;
    lastA = HIGH;
    
    // Inicializar variables del menú
    modoMenu = false;
    opcionMenu = 0;
    lastBtn = HIGH;
    ultimoPresionBtn = 0;
    
    // Inicializar variables para control manual
    tempManual = false;
    humedadManual = false;
    tempRefManual = 25; // Valor por defecto
    humedadUmbralManual = 50; // Valor por defecto
    
    // Inicializar variables para control manual de sistemas
    ventilacionManual = false;
    riegoManual = false;
    ventilacionForzado = false;
    riegoForzado = false;
    
    // Inicializar variables para navegación en control manual
    sistemaSeleccionado = 0; // Empezar en ventilación
    modoVentilacion = 0; // AUTO
    modoRiego = 0; // AUTO
}

void Device::begin()
{
    dht.begin();
    pinMode(ledVerde, OUTPUT);

    pinMode(encA, INPUT_PULLUP);
    pinMode(encB, INPUT_PULLUP);
    pinMode(encBtn, INPUT_PULLUP);

    humedadUmbral = 40 + esp_random() % 21; // Umbral aleatorio

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
    if (modoMenu)
    {
        handleMenu();
        showMenu();
    }
    else
    {
        // Solo detectar entrada al menú cuando NO estamos en modo menú
        int btn = digitalRead(encBtn);
        if (btn != lastBtn && btn == LOW && (millis() - ultimoPresionBtn) > DEBOUNCE_DELAY)
        {
            ultimoPresionBtn = millis();
            modoMenu = true;
            Serial.println("Entrando al MENU");
        }
        lastBtn = btn;

        handleEncoder(); // Lee el estado del encoder y cambia la pantalla si se gira
        temp = dht.readTemperature(); // Lee la temperatura actual del sensor 
        hum = dht.readHumidity(); // Lee la humedad
        
        // Usar valor manual o del potenciómetro según el modo
        if (tempManual) {
            refTemp = tempRefManual;
        } else {
            refTemp = map(analogRead(potPin), 0, 4095, 15, 35); // Lee el valor del potenciómetro
        }
        
        // Usar umbral manual o el aleatorio según el modo
        if (humedadManual) {
            humedadUmbral = humedadUmbralManual;
        }
        // Si no está en modo manual, mantiene el umbral aleatorio original

        // si esta seleccionada la pantalla de ventilacion la controla, en su defecto controla la de riego
        if (pantalla == 0)  
            controlVentilacion();
        else
            controlRiego();
        showScreen();
    }
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
    if (ventilacionManual) {
        // Modo manual: usar estado forzado
        if (ventilacionForzado != ventiladorActivo) {
            ventiladorActivo = ventilacionForzado;
            Serial.print("Ventilacion MANUAL: ");
            Serial.println(ventiladorActivo ? "ON" : "OFF");
        }
        digitalWrite(ledVerde, ventiladorActivo ? HIGH : LOW);
    }
    else {
        // Modo automático: usar lógica de temperatura
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
}

void Device::controlRiego()
{
    if (riegoManual) {
        // Modo manual: usar estado forzado
        if (riegoForzado != riegoActivo) {
            riegoActivo = riegoForzado;
            Serial.print("Riego MANUAL: ");
            Serial.println(riegoActivo ? "ON" : "OFF");
        }
        
        if (riegoActivo) {
            // En modo manual ON: hacer parpadear el LED
            if (millis() - ultimoParpadeo > 500)
            {
                ultimoParpadeo = millis();
                estadoLedRiego = !estadoLedRiego;
                digitalWrite(ledVerde, estadoLedRiego);
            }
        } else {
            // En modo manual OFF: LED apagado
            digitalWrite(ledVerde, LOW);
        }
    }
    else {
        // Modo automático: usar lógica de humedad
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

void Device::handleMenu()
{
    int A = digitalRead(encA);
    int btn = digitalRead(encBtn);
    
    // Navegación por encoder
    if (A != lastA && A == LOW) // Detecta rotación del encoder
    {
        if (digitalRead(encB) == LOW)
        {
            opcionMenu = (opcionMenu + 1) % TOTAL_OPCIONES_MENU; // Giro derecha
        }
        else
        {
            opcionMenu = (opcionMenu - 1 + TOTAL_OPCIONES_MENU) % TOTAL_OPCIONES_MENU; // Giro izquierda
        }
        Serial.print("Opcion menu: ");
        Serial.println(opcionMenu);
    }
    lastA = A;
    
    // Detectar selección de opción - usando variable separada para el menú
    static bool lastBtnMenu = HIGH;
    static unsigned long ultimoPresionBtnMenu = 0;
    
    if (btn != lastBtnMenu && btn == LOW && (millis() - ultimoPresionBtnMenu) > DEBOUNCE_DELAY)
    {
        ultimoPresionBtnMenu = millis();
        Serial.print("Ejecutando opcion: ");
        Serial.println(opcionMenu + 1);
        
        switch(opcionMenu)
        {
            case 0:
                // Mostrar estado completo - leer sensores y aplicar lógica de valores manuales
                temp = dht.readTemperature(); 
                hum = dht.readHumidity(); 
                
                // Usar la misma lógica que en update() para respetar valores manuales
                if (tempManual) {
                    refTemp = tempRefManual;
                } else {
                    refTemp = map(analogRead(potPin), 0, 4095, 15, 35);
                }
                
                // Usar umbral manual o el aleatorio según el modo
                if (humedadManual) {
                    humedadUmbral = humedadUmbralManual;
                }
                
                showEstadoCompleto();
                // Esperar a que se presione el botón para volver
                while(digitalRead(encBtn) == HIGH) {
                    delay(50);
                }
                while(digitalRead(encBtn) == LOW) {
                    delay(50);
                }
                break;
            case 1:
                Serial.println("=== MODIFICAR VALORES ===");
                modificarValores();
                break;
            case 2:
                Serial.println("== CONTROL MANUAL ==");
                Serial.println("Usar encoder para navegar y cambiar estados");
                controlManual();
                break;
            case 3:
                Serial.println("Saliendo del MENU");
                modoMenu = false;
                pantalla = 0; // Resetear a pantalla principal
                break;
        }
    }
    lastBtnMenu = btn;
}

void Device::showMenu()
{
    display->clearDisplay();
    display->setCursor(0, 0);
    display->setTextSize(1);
    display->println("=== MENU OPCIONES ===");
    display->println();
    
    // Opción 0
    if (opcionMenu == 0) display->print("> ");
    else display->print("  ");
    display->println("1. Estado completo");
    
    // Opción 1
    if (opcionMenu == 1) display->print("> ");
    else display->print("  ");
    display->println("2. Modificar valores");
    
    // Opción 2
    if (opcionMenu == 2) display->print("> ");
    else display->print("  ");
    display->println("3. Control manual");
    
    // Opción 3 - Nueva opción para salir
    if (opcionMenu == 3) display->print("> ");
    else display->print("  ");
    display->println("4. Salir");
    
    display->println();
    display->println("Girar: Navegar");
    display->println("Presionar: Seleccionar");
    
    display->display();
}

void Device::showEstadoCompleto()
{
    display->clearDisplay();
    display->setCursor(0, 0);
    display->setTextSize(1);
    display->println("= ESTADO COMPLETO =");
    display->println();
    display->print("Temp: ");
    display->print(temp);
    display->print("C (Ref:");
    display->print(refTemp);
    display->println("C)");
    display->print("Hum: ");
    display->print(hum);
    display->print("% (Umb:");
    display->print(humedadUmbral);
    display->println("%)");
    display->println();
    display->print("Ventilacion: ");
    display->println(ventiladorActivo ? "ON" : "OFF");
    display->print("Riego: ");
    display->println(riegoActivo ? "ON" : "OFF");
    display->println();
    display->println("Presionar para volver");
    display->display();
}

void Device::modificarValores()
{
    Serial.println("\n=== MODIFICAR VALORES DE REFERENCIA ===");
    Serial.println("1. Temperatura de referencia");
    Serial.println("2. Umbral de humedad");
    Serial.println("3. Volver al menu");
    Serial.print("Seleccione opcion (1-3): ");
    
    // Mostrar menú en pantalla también
    display->clearDisplay();
    display->setCursor(0, 0);
    display->setTextSize(1);
    display->println("=MODIFICAR VALORES=");
    display->println();
    display->println("1. Temp referencia");
    display->println("2. Umbral humedad");
    display->println("3. Volver");
    display->println();
    display->println("Usar Monitor Serie");
    display->println("para seleccionar");
    display->display();
    
    // Esperar entrada del usuario
    while (!Serial.available()) {
        delay(100);
    }
    
    int opcion = Serial.parseInt();
    while (Serial.available()) Serial.read(); // Limpiar buffer
    
    Serial.println(opcion);
    
    switch(opcion) {
        case 1:
            {
                Serial.println("\n--- TEMPERATURA DE REFERENCIA ---");
                Serial.print("Valor actual: ");
                Serial.print(tempManual ? tempRefManual : refTemp);
                Serial.println("C");
                Serial.print("Modo actual: ");
                Serial.println(tempManual ? "MANUAL" : "AUTOMATICO (potenciometro)");
                Serial.println("\nOpciones:");
                Serial.println("A. Activar modo manual (ingresar nuevo valor)");
                Serial.println("B. Activar modo automatico (potenciometro)");
                Serial.print("Seleccione (A/B): ");
                
                while (!Serial.available()) delay(100);
                char subOpcion = Serial.read();
                while (Serial.available()) Serial.read();
                Serial.println(subOpcion);
                
                if (subOpcion == 'A' || subOpcion == 'a') {
                    Serial.print("Ingrese temperatura de referencia (15-35C): ");
                    while (!Serial.available()) delay(100);
                    int nuevaTemp = Serial.parseInt();
                    while (Serial.available()) Serial.read();
                    Serial.println(nuevaTemp);
                    
                    if (nuevaTemp >= 15 && nuevaTemp <= 35) {
                        tempRefManual = nuevaTemp;
                        tempManual = true;
                        Serial.println("Modo MANUAL activado para temperatura");
                        Serial.print("Temperatura de referencia configurada: ");
                        Serial.print(tempRefManual);
                        Serial.println("C");
                    } else {
                        Serial.println("ERROR: Temperatura debe estar entre 15-35C");
                        Serial.println("Manteniendo modo automatico");
                    }
                }
                else if (subOpcion == 'B' || subOpcion == 'b') {
                    tempManual = false;
                    Serial.println("Modo AUTOMATICO activado - usando potenciometro");
                }
                else {
                    Serial.println("Opcion invalida. Use A o B");
                }
            }
            break;
            
        case 2:
            {
                Serial.println("\n--- UMBRAL DE HUMEDAD ---");
                Serial.print("Valor actual: ");
                Serial.print(humedadUmbral);
                Serial.println("%");
                Serial.print("Modo actual: ");
                Serial.println(humedadManual ? "MANUAL" : "AUTOMATICO (aleatorio)");
                Serial.println("\nOpciones:");
                Serial.println("A. Activar modo manual (ingresar nuevo valor)");
                Serial.println("B. Activar modo automatico (usar umbral aleatorio)");
                Serial.print("Seleccione (A/B): ");
                
                while (!Serial.available()) delay(100);
                char subOpcion = Serial.read();
                while (Serial.available()) Serial.read();
                Serial.println(subOpcion);
                
                if (subOpcion == 'A' || subOpcion == 'a') {
                    Serial.print("Ingrese umbral de humedad (20-80%): ");
                    while (!Serial.available()) delay(100);
                    int nuevoUmbral = Serial.parseInt();
                    while (Serial.available()) Serial.read();
                    Serial.println(nuevoUmbral);
                    
                    if (nuevoUmbral >= 20 && nuevoUmbral <= 80) {
                        humedadUmbralManual = nuevoUmbral;
                        humedadUmbral = nuevoUmbral;
                        humedadManual = true;
                        Serial.println("Modo MANUAL activado para humedad");
                        Serial.print("Umbral de humedad configurado: ");
                        Serial.print(humedadUmbral);
                        Serial.println("%");
                    } else {
                        Serial.println("ERROR: Umbral debe estar entre 20-80%");
                        Serial.println("Manteniendo modo automatico");
                    }
                }
                else if (subOpcion == 'B' || subOpcion == 'b') {
                    humedadManual = false;
                    // Mantener el umbral aleatorio original
                    Serial.println("Modo AUTOMATICO activado - usando umbral aleatorio original");
                }
                else {
                    Serial.println("Opcion invalida. Use A o B");
                }
            }
            break;
            
        case 3:
            Serial.println("Volviendo al menu principal...");
            break;
            
        default:
            Serial.println("Opcion invalida");
            break;
    }
    
    Serial.println("\nPresione cualquier tecla para continuar...");
    while (!Serial.available()) delay(100);
    while (Serial.available()) Serial.read();
}

void Device::showControlManual()
{
    display->clearDisplay();
    display->setCursor(0, 0);
    display->setTextSize(1);
    display->println("=== CONTROL MANUAL ===");
    display->println();
    
    // Mostrar ventilación
    if (sistemaSeleccionado == 0) display->print("> ");
    else display->print("  ");
    display->print("Ventilacion: ");
    switch(modoVentilacion) {
        case 0: display->println("AUTO"); break;
        case 1: display->println("ON"); break;
        case 2: display->println("OFF"); break;
    }
    
    // Mostrar riego
    if (sistemaSeleccionado == 1) display->print("> ");
    else display->print("  ");
    display->print("Riego: ");
    switch(modoRiego) {
        case 0: display->println("AUTO"); break;
        case 1: display->println("ON"); break;
        case 2: display->println("OFF"); break;
    }
    
    display->println();
    display->println("Estado actual:");
    display->print("Ventilador: ");
    display->println(ventiladorActivo ? "ON" : "OFF");
    display->print("Riego: ");
    display->println(riegoActivo ? "ON" : "OFF");
    
    display->println();
    display->println("Girar: Navegar");
    display->println("Presionar: Cambiar");
    display->println("Mantener: Salir");
    
    display->display();
}

void Device::controlManual()
{
    // Variables locales para manejar encoder en control manual
    static bool lastBtnControl = HIGH;
    static unsigned long ultimoPresionBtnControl = 0;
    bool salirControlManual = false;
    
    // Mostrar instrucciones iniciales
    Serial.println("Navegacion: Girar encoder");
    Serial.println("Cambiar modo: Presionar encoder");
    Serial.println("Salir: Mantener presionado encoder por 2 segundos");
    
    while (!salirControlManual) {
        // Leer estado del encoder
        int A = digitalRead(encA);
        int btn = digitalRead(encBtn);
        
        // Navegación por encoder (cambiar entre ventilación y riego)
        if (A != lastA && A == LOW) {
            if (digitalRead(encB) == LOW) {
                sistemaSeleccionado = (sistemaSeleccionado + 1) % 2; // 0 o 1
            } else {
                sistemaSeleccionado = (sistemaSeleccionado - 1 + 2) % 2; // 0 o 1
            }
            Serial.print("Sistema seleccionado: ");
            Serial.println(sistemaSeleccionado == 0 ? "VENTILACION" : "RIEGO");
        }
        lastA = A;
        
        // Detectar presión del botón
        if (btn != lastBtnControl && btn == LOW && (millis() - ultimoPresionBtnControl) > DEBOUNCE_DELAY) {
            ultimoPresionBtnControl = millis();
            
            if (sistemaSeleccionado == 0) {
                // Cambiar modo de ventilación: AUTO -> ON -> OFF -> AUTO
                modoVentilacion = (modoVentilacion + 1) % 3;
                Serial.print("Ventilacion: ");
                switch(modoVentilacion) {
                    case 0: 
                        ventilacionManual = false;
                        Serial.println("AUTO");
                        break;
                    case 1:
                        ventilacionManual = true;
                        ventilacionForzado = true;
                        Serial.println("FORZADO ON");
                        break;
                    case 2:
                        ventilacionManual = true;
                        ventilacionForzado = false;
                        Serial.println("FORZADO OFF");
                        break;
                }
            } else {
                // Cambiar modo de riego: AUTO -> ON -> OFF -> AUTO
                modoRiego = (modoRiego + 1) % 3;
                Serial.print("Riego: ");
                switch(modoRiego) {
                    case 0:
                        riegoManual = false;
                        Serial.println("AUTO");
                        break;
                    case 1:
                        riegoManual = true;
                        riegoForzado = true;
                        Serial.println("FORZADO ON");
                        break;
                    case 2:
                        riegoManual = true;
                        riegoForzado = false;
                        Serial.println("FORZADO OFF");
                        break;
                }
            }
        }
        lastBtnControl = btn;
        
        // Detectar presión larga para salir (2 segundos)
        static unsigned long inicioPresion = 0;
        if (btn == LOW) {
            if (inicioPresion == 0) {
                inicioPresion = millis();
            } else if (millis() - inicioPresion > 2000) {
                salirControlManual = true;
                Serial.println("Saliendo del control manual...");
            }
        } else {
            inicioPresion = 0;
        }
        
        // Mostrar pantalla de control manual
        showControlManual();
        
        delay(100); // Pequeña pausa para evitar lecturas muy rápidas
    }
}
