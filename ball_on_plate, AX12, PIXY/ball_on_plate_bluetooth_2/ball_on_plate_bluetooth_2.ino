#include <BluetoothSerial.h>
#include <SPI.h>
#include <Pixy.h>
#include "AX12A.h"
#include <PID_v1.h>

#define DirectionPin   (2u)
#define BaudRate      (1000000ul)
#define ID1       (1u) //X
#define ID2       (4u) //Y

int xmaxi = 270;
int xmini = 70;
int ymaxi = 190;
int ymini = 10;

int j;
int xkalibrator = 665; // ID1
int ykalibrator = 665; // ID2

double xSetpoint, xInput, xOutput, xServoSignal;
double ySetpoint, yInput, yOutput, yServoSignal;


float xKp = 1.1;   // 1.1
float xKi = 0.61;  //.05
float xKd = 0.66;   //.2

float yKp = 1.1;   //.9
float yKi = 0.61;  // .05
float yKd = 0.66;   //.2

PID xmyPID(&xInput, &xOutput, &xSetpoint, xKp, xKi, xKd, DIRECT);
PID ymyPID(&yInput, &yOutput, &ySetpoint, yKp, yKi, yKd, DIRECT);

Pixy nyaku;
BluetoothSerial SerialBT;

unsigned long previousMillis = 0;
const unsigned long interval = 1000;

void setup()
{
  xSetpoint = 180;//  Sumbu X
  ySetpoint = 105; // sumbu Y
  Serial.begin(115200);
  SerialBT.begin("ESP32"); // Nama perangkat Bluetooth yang akan terlihat oleh perangkat lain
  ax12a.begin(BaudRate, DirectionPin, &Serial2);
  nyaku.init();
  Serial.println("Siap...");

  xmyPID.SetMode(AUTOMATIC);
  xmyPID.SetOutputLimits(-200, 200);
  ymyPID.SetMode(AUTOMATIC);
  ymyPID.SetOutputLimits(-200, 200);
}

void loop() {
  static int i = 0;
  uint16_t blocks;
  char buf[32];
  blocks = nyaku.getBlocks();
  if (blocks) {
    i++;
    if (i % 1 == 0) {
      for (j = 0; j < blocks; j++) {
        loopa();
      }
    }
  }

  // Menerima data Bluetooth dan mengubah nilai Kp, Ki, Kd
  if (SerialBT.available()) {
    String data = SerialBT.readStringUntil('\n');
    if (data.length() > 0) {
      // Memisahkan data menjadi array dengan pemisah koma
      float values[8];
      int index = 0;
      char *ptr = strtok((char *)data.c_str(), ",");
      while (ptr != NULL && index < 8) {
        values[index++] = atof(ptr);
        ptr = strtok(NULL, ",");
      }

      // Mengganti nilai Kp, Ki, Kd
      if (index == 8) {
        xKp = values[0];
        xKi = values[1];
        xKd = values[2];
        yKp = values[3];
        yKi = values[4];
        yKd = values[5];
        xSetpoint = values[6];//  Sumbu X
        ySetpoint = values[7];//  Sumbu Y'

        String dataI = String(xSetpoint) + "," + String(ySetpoint) + "\n";
        SerialBT.print(dataI);

        xmyPID.SetTunings(xKp, xKi, xKd);
        ymyPID.SetTunings(yKp, yKi, yKd);
      }
    }
  }
  // Pengiriman data setiap 1 detik
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    String dataI = "X :" + String(xSetpoint) + ", Y : " + String(ySetpoint) + "\n";
    SerialBT.print(dataI);
  }
}

void loopa() {
  if (nyaku.blocks[j].x <= xmaxi) {
    xInput = nyaku.blocks[j].x;
    yInput = nyaku.blocks[j].y;
    if (xInput > xmaxi) {
      xInput = xmaxi;
    }
    if (xInput < xmini) {
      xInput = xmini;
    }

    xmyPID.Compute();
    xServoSignal = xkalibrator + xOutput; // +/- tergantung posisi tungkai pada servo
    ax12a.move(ID1, xServoSignal);
    ymyPID.Compute();
    yServoSignal = ykalibrator - yOutput; // +/- tergantung posisi tungkai pada servo
    ax12a.move(ID2, yServoSignal);



    Serial.println("Posisi x dan y : ");
    //    Serial.print(xKd);
    //    Serial.print(", ");
    //    Serial.print(xKi);
    //    Serial.print(", ");
    //    Serial.print(xKd);
    //    Serial.print(", ");
    //    Serial.print(yKp);
    //    Serial.print(", ");
    //    Serial.print(yKi);
    //    Serial.print(", ");
    //    Serial.print(yKd);
    //    Serial.print(", X ");
    //    Serial.print(xSetpoint);
    //    Serial.print(", Y ");
    //    Serial.print(ySetpoint);
    Serial.println(",");
    Serial.print(xSetpoint);
    Serial.print(",");
    Serial.print(xInput);
    Serial.print(",");
    Serial.print(ySetpoint);
    Serial.print(",");
    Serial.print(yInput);
    Serial.print(",");
    Serial.print(xServoSignal);
    Serial.print(",");
    Serial.println(yServoSignal);
  }
}
