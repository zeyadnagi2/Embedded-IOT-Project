#include <ESP32Servo.h>

Servo Myservo;
int pin = 12;

void setup() {
  // put your setup code here, to run once:
  Myservo.attach(pin);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');

    if (command == "o") {
      Myservo.write(170);
    } else if (command == "c") {
      Myservo.write(80);
    }
  }

  delay(20);
}
