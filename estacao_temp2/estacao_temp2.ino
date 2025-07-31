#define BLYNK_TEMPLATE_ID "TMPL2BZrTOFWQ"
#define BLYNK_TEMPLATE_NAME "Estação Incêndio"
#define BLYNK_AUTH_TOKEN "k6E-R1fooMAq7YnTBmGBAj9N8S8apvj_"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoBold18pt7b.h>

// oled
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define MQ2_PIN 34
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Ícones desenhados por primitivas
void drawTherm(int x, int y) {
  display.drawRoundRect(x, y, 6, 16, 2, WHITE);
  display.fillCircle(x+3, y+16, 4, WHITE);
}
void drawDrop(int x, int y) {
  display.fillTriangle(x+4, y, x, y+10, x+8, y+10, WHITE);
  display.fillCircle(x+4, y+10, 4, WHITE);
}
void drawFlame(int x, int y) {
  display.fillTriangle(x+4, y,   x,   y+12, x+8, y+12, WHITE);
  display.fillTriangle(x+4, y+4, x+2, y+12, x+6, y+12, WHITE);
}

// Definindo o pino do DHT11
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Wi-Fi e Blynk
char ssid[] = "xxx";
char pass[] = "xxxxxxx";

BlynkTimer timer;

void setup() {
  Serial.begin(115200);
  dht.begin();

  Serial.println("Conectando ao Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Conectado ao Blynk!");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Falha ao iniciar o OLED"));
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Estacao Online!");
  display.display();

  // Agora o intervalo é de 300 000 ms = 5 minutos
  timer.setInterval(300000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}

void sendSensorData() {
  float h      = dht.readHumidity();
  float t      = dht.readTemperature();
  int   mq2v   = analogRead(MQ2_PIN);
  float mq2Pct = (mq2v / 4095.0) * 100.0;

  // Se der erro no DHT11
  if (isnan(h) || isnan(t)) {
    Serial.println("Falha na leitura do sensor DHT11!");
    display.clearDisplay();
    display.setFont();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.println("Erro no DHT11!");
    display.setCursor(0, 32);
    display.println("Verifique conexao");
    display.display();
    return;
  }

  display.clearDisplay();

  // — Termômetro + Temp
  drawTherm(0, 0);
  display.setFont();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.print((int)t);
  display.write(247);  // °
  display.print("C");

  // — Gota + Umid
  drawDrop(0, 22);
  display.setTextSize(2);
  display.setCursor(10, 22);
  display.print((int)h);
  display.print("%");

  // — Chama + Gas%
  drawFlame(0, 44);
  display.setTextSize(1);
  display.setCursor(10, 48);
  display.print("Gas ");
  display.print((int)mq2Pct);
  display.print("%");

  // — Barra de progresso de gás
  int barW = map((int)mq2Pct, 0, 100, 0, SCREEN_WIDTH - 10);
  display.drawRect(10, 58, SCREEN_WIDTH - 10, 6, WHITE);
  display.fillRect(10, 58, barW, 6, WHITE);

  display.display();

  // Envia 3 mensagens ao Blynk (h, t, gás)
  Blynk.virtualWrite(V0, h);
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, mq2Pct);

  Serial.printf("T=%.1fC H=%.1f%% G=%.1f%%\n", t, h, mq2Pct);
}
