#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>
#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

#define SERVER_IP "172.16.17.106"
#define STASSID "test"
#define STAPSK "12345678"


void setup() {
  Serial.begin(115200);
  Serial.println(F("DHT22 test!"));
  dht.begin();

  Serial.println();
  Serial.println();
  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  float Humidity = dht.readHumidity();
  float Temperature = dht.readTemperature();
  
  WiFiClient client;

  const int httpPort = 80;
  if (!client.connect(SERVER_IP, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  client.print(String("GET http://172.16.17.106/IOT_ASER/DHT22GET.php?") +
               ("&temperature=") + Temperature +
               ("&humidity=") + Humidity +
               " HTTP/1.1\r\n" +
               "Host: " + SERVER_IP + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 1000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  Serial.println(Temperature);
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);

  }

  Serial.println();
  Serial.println("closing connection");


}
