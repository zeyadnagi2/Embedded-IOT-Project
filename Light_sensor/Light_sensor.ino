#define DO_PIN 35  // ESP32's pin GPIO13 connected to DO pin of the ldr module

void setup() {
  // initialize serial communication
  Serial.begin(115200);
  // initialize the ESP32's pin as an input
  pinMode(DO_PIN, INPUT);
}

void loop() {
  int lightState = digitalRead(DO_PIN);

  if (lightState == LOW){
    Serial.println("It is light");
  }

  delay(1000);
}
