int DO_PIN = 14;  // ESP32's pin GPIO13 connected to DO pin of the flame sensor

void setup() {
  // initialize serial communication
  Serial.begin(115200);
  // initialize the ESP32's pin as an input
  pinMode(DO_PIN, INPUT);
}

void loop() {
  int flame_state = digitalRead(DO_PIN);

  if (flame_state == HIGH){
    Serial.println("Flame dected => The fire is detected");
  }
  
  delay(1000);
}
