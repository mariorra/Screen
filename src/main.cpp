// Pines para la pantalla y la tarjeta SD
#define TFT_CS 15
#define TFT_DC 33
#define TFT_RST -1
#define SD_CS 14 // GPIO 14 para la tarjeta SD
#define TSC2007_ADDR 0x48 // Dirección I2C del controlador táctil
#define TSC2007_IRQ -1    // No usamos pin de interrupción
#define BUZZER_PIN 27


#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_TSC2007.h>
#include <Wire.h>
#include <SD.h>

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
Adafruit_TSC2007 touch;

// Variables globales
float  Ttarget = 25.0; // Temperatura objetivo inicial

// Prototipos de funciones
void displayBMP(const char *filename, int16_t x, int16_t y);
void printCenteredText(const char *text, int y, uint16_t color, uint8_t size);
void drawUI(void);
void drawButton(int x, int y, int w, int h, const char *label, uint16_t color);
void updateTtargetDisplay(void);
void drawMinMaxMessages(void) ;
void handleTouch(int x, int y);
void beep(int frequency, int duration);


void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT); 
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
  beep(1000,250);
    Serial.println("SETUP 1");
  // Inicializar pantalla
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  Serial.println(" sETUP 2");
  // Inicializar tarjeta SD
  if (!SD.begin(SD_CS)) {
    Serial.println("Error: No se pudo inicializar la tarjeta SD.");
    while (1);
  }

  // Inicializar táctil
  if (!touch.begin(TSC2007_ADDR)) {
    Serial.println("Error: No se detecta pantalla táctil.");
    while (1);
  }

  // Mostrar imagen de fondo
  displayBMP("/roche.bmp", 0, 0);

  // Dibujar UI inicial
  drawUI();
}

void loop() {
  TS_Point p = touch.getPoint();
  // Verificar si el toque es válido (presión detectada)
  if (p.z > 0) { // `p.z` es el nivel de presión
     // Aquí puedes manejar interacciones con la pantalla
    handleTouch(p.x, p.y);
    delay(1000);
  }
}

// Dibujar la interfaz de usuario
void drawUI() {
  // Agregar textos en la parte inferior
  printCenteredText("Mario Ruiz", 170, ILI9341_BLUE, 2);
  printCenteredText("Interview", 200, ILI9341_BLUE, 2);
  printCenteredText("T1 23.5C T2 24.0C", 230, ILI9341_BLUE, 2);
  printCenteredText("PWM: 75%", 290, ILI9341_BLUE, 2);

  // Dibujar botones interactivos
  drawButton(0, 0, 40, 40, "-", ILI9341_RED);    // Botón "-"
  drawButton(200, 0, 40, 40, "+", ILI9341_DARKGREEN); // Botón "+"

  // Mostrar Ttarget en el centro
  updateTtargetDisplay();
   // Dibujar mensajes de mínimo y máximo
  drawMinMaxMessages();
}

// Dibujar la interfaz de usuario
void drawMinMaxMessages() {
  // Mensaje para el mínimo (bajo el botón '-')
  tft.fillRect(0, 45, 50, 12, ILI9341_WHITE); // Limpiar área
  tft.setCursor(2, 45);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(1);
  tft.println("MIN 20");

  // Mensaje para el máximo (bajo el botón '+')
  tft.fillRect(200, 45, 50, 20, ILI9341_WHITE); // Limpiar área
  tft.setCursor(200, 45);
  tft.setTextColor(ILI9341_DARKGREEN);
  tft.setTextSize(1);
  tft.println("MAX 30");
}

// Dibujar un botón rectangular con texto
void drawButton(int x, int y, int w, int h, const char *label, uint16_t color) {
  tft.fillRect(x, y, w, h, color);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  int textX = x + (w - (strlen(label) * 12)) / 2;
  int textY = y + (h - 16) / 2;
  tft.setCursor(textX, textY);
  tft.println(label);
}

// Actualizar la visualización de Ttarget
void updateTtargetDisplay() {
  char buffer[32];
  Serial.printf("%.1f% \n",Ttarget);
  sprintf(buffer, "T %.1f C", Ttarget); // Formatear con dos decimales
  Serial.printf("%.1f% \n",Ttarget);
  tft.fillRect(60, 260, 120, 20, ILI9341_WHITE); // Borrar la pantalla anterior
  printCenteredText(buffer, 260, ILI9341_BLUE, 2); // Mostrar el nuevo valor
  beep(1000,100);
}


// Procesar toques en pantalla
void handleTouch(int x, int y) {
// Mapear las coordenadas táctiles a la resolución de la pantalla
  int mappedX = map(x, 0, 4096, 0, 240); // Ajustar X
  int mappedY = map(y, 0, 4096, 0, 320); // Ajustar Y

  // Depuración para verificar las coordenadas normalizadas
  Serial.print("Toque normalizado en: X=");
  Serial.print(mappedX);
  Serial.print(", Y=");
  Serial.println(mappedY);

  // Detectar botón "-" (izquierda), con margen de 15 píxeles
  if (mappedX > 0 && mappedX < 75 && mappedY > 0 && mappedY < 75) {
    Serial.println("Botón '-' detectado");
    if (Ttarget > 20.00) { // No permitir menos de 20
      Ttarget -= 0.50;
      updateTtargetDisplay(); // Actualiza la pantalla
    }
  }
  // Detectar botón "+" (derecha), con margen de 15 píxeles
  else if (mappedX > 185 && mappedX < 255 && mappedY > 0 && mappedY < 55) {
    Serial.println("Botón '+' detectado");
    if (Ttarget < 30.00) { // No permitir más de 30
      Ttarget += 0.50;
      updateTtargetDisplay(); // Actualiza la pantalla
    }
  } else {
    Serial.println("No se detectó ningún botón válido.");
  }
}


// Mostrar imagen BMP desde la tarjeta SD
void displayBMP(const char *filename, int16_t x, int16_t y) {
  File bmpFile = SD.open(filename);
  if (!bmpFile) {
    Serial.println("Error: No se pudo abrir el archivo BMP.");
    return;
  }

  uint8_t bmpHeader[54];
  bmpFile.read(bmpHeader, 54);

  uint32_t bmpWidth = *(int32_t *)&bmpHeader[18];
  uint32_t bmpHeight = *(int32_t *)&bmpHeader[22];
  uint16_t bmpDepth = *(int16_t *)&bmpHeader[28];

  if (bmpDepth != 24) {
    Serial.println("Error: El archivo BMP debe ser de 24 bits.");
    bmpFile.close();
    return;
  }

  bmpFile.seek(54);
  uint8_t buffer[bmpWidth * 3];

  for (int row = 0; row < bmpHeight; row++) {
    bmpFile.read(buffer, bmpWidth * 3);
    for (int col = 0; col < bmpWidth; col++) {
      uint16_t color = tft.color565(buffer[col * 3 + 2], buffer[col * 3 + 1], buffer[col * 3]);
      tft.drawPixel(x + col, y + (bmpHeight - 1 - row), color);
    }
  }

  bmpFile.close();
  Serial.println("Imagen mostrada correctamente.");
}

// Centrar texto horizontalmente
void printCenteredText(const char *text, int y, uint16_t color, uint8_t size) {
  int16_t x1, y1;
  uint16_t w, h;

  tft.setTextSize(size);
  tft.setTextColor(color);
  tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  int16_t x = (240 - w) / 2; // Centrar horizontalmente
  tft.setCursor(x, y);
  tft.println(text);
}


void beep(int frequency, int duration) {
  tone(BUZZER_PIN, frequency, duration); // Genera una señal en el buzzer
  delay(duration);                      // Espera la duración del tono
  noTone(BUZZER_PIN); 
}