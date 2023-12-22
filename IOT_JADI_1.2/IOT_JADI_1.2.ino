#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SS_PIN 5
#define RST_PIN 15
#define LED_BUILTIN 14

// const char* ssid = "pastori";
// const char* password = "tulangow";


const char* ssid = "aser";
const char* password = "aser4532";

TaskHandle_t Task2;
bool buka;
String tam = "";
int previousMillis = 0;
int interval = 5000;

MFRC522 mfrc522(SS_PIN, RST_PIN);
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID reader is ready!");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  tampilan(tam = "  RFID DOOR LOCK", tam = "  silahkan sken");

  WiFi.begin(ssid, password);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("");

  xTaskCreatePinnedToCore(
    Task2code, /* Task function. */
    "Task2",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task2,    /* Task handle to keep track of created task */
    1);        /* pin task to core 1 */
}

void loop() {

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {

    String uidData = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uidData += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") + String(mfrc522.uid.uidByte[i], HEX);
    }
    addToDataBuffer(uidData);

    Serial.println();
    mfrc522.PICC_HaltA();
  }
  display.display();
}

void Task2code(void* pvParameters) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    static unsigned long lastDataSendTime = 0;
    unsigned long currentTime = millis();

    digitalWrite(LED_BUILTIN, 0);
    if (buka != 0) {
      Serial.print("1 ");
      digitalWrite(LED_BUILTIN, 1);
      delay(3000);
      digitalWrite(LED_BUILTIN, 0);
      delay(100);
      Serial.println("0 ");
    }
    buka = 0;
  }
}

void addToDataBuffer(String uid) {

  Serial.println("UID: " + uid);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(5, 1);
  display.println("Dalam proses");
  display.setCursor(5, 20);
  display.print("UID: " + uid);
  display.display();

  String result = sendDataToGoogle(uid);

  if (result != "0") {
    tampilan("UID: " + uid, result);\
    buka = 1;
  } else {
    tam = "UID tidak terdaftar";
    tampilan("UID: " + uid, tam);
    buka = 0;
  }
}

// void buka() {
//   unsigned long currentMillis = millis();
//   if (currentMillis - previousMillis >= interval) {
//     previousMillis = currentMillis;
//     delay(5000);
//     digitalWrite(LED_BUILTIN, 1);
//     Serial.print("1");
//   } else {
//     digitalWrite(LED_BUILTIN, 0);
//     Serial.print("0");
//   }
// }

String sendDataToGoogle(String uid) {
  WiFiClient client;
  HTTPClient http;

  http.begin("http://192.168.246.114/BLAJAR_IOT/get_data_server.php");
  // http.addHeader("Content-Type", "application/json");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST("uid=" + uid);
  delay(100);

  String res = "0";
  if (httpResponseCode > 0) {
    res = http.getString();

  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  delay(1000);
  return res;
}

void tampilan(String tam1, String tam2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(5, 1);
  display.println(tam1);
  display.println(" ");
  display.setCursor(5, 20);
  display.print(tam2);
}
