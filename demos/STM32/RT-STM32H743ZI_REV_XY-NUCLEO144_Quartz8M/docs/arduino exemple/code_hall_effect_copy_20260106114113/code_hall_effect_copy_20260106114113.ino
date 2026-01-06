#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ========= OLED ========= */
#define OLED_MOSI PI3
#define OLED_CLK  PI1
#define OLED_DC   PI5
#define OLED_RST  PI4
#define OLED_CS   PI6

// SPI SOFTWARE
Adafruit_SSD1306 display(
  128, 64,
  OLED_MOSI,
  OLED_CLK,
  OLED_DC,
  OLED_RST,
  OLED_CS
);

/* ========= MUX ========= */
#define MUX_S0 PA5
#define MUX_S1 PA4
#define MUX_S2 PA6

/* ========= ADC ========= */
#define ADC_A PC4   // ADC channel 4
#define ADC_B PA7   // ADC channel 7

void setMux(uint8_t ch) {
  digitalWrite(MUX_S0, (ch >> 0) & 1);
  digitalWrite(MUX_S1, (ch >> 1) & 1);
  digitalWrite(MUX_S2, (ch >> 2) & 1);
}

void setup() {
  delay(500);

  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);

  digitalWrite(MUX_S0, 0);
  digitalWrite(MUX_S1, 0);
  digitalWrite(MUX_S2, 0);

  analogReadResolution(12); // 0..4095

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("HALL MUX TEST");
  display.display();

  delay(1000);
}

void loop() {
  display.clearDisplay();
  display.setCursor(0, 0);

  display.println("HALL MUX TEST");

  for (int ch = 0; ch < 8; ch++) {
    setMux(ch);
    delayMicroseconds(10);

    int a = analogRead(ADC_A);
    int b = analogRead(ADC_B);

    display.print("CH");
    display.print(ch);
    display.print(": ");
    display.print(a);
    display.print(" ");
    display.println(b);
  }

  display.display();
  delay(100);
}
