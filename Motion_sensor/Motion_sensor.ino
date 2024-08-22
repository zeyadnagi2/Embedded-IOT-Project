#define SENSOR_PIN 33 // ESP32 pin GPIO18 connected to OUT pin of IR obstacle avoidance sensor

int lastState = HIGH;  // the previous state from the input pin
int currentState;      // the current reading from the input pin

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
  // initialize the ESP32's pin as an input
  pinMode(SENSOR_PIN, INPUT);
}

void loop() {
  currentState = digitalRead(SENSOR_PIN);
  Serial.print("Current State: ");
  Serial.println(currentState);

  delay(1000);
  lastState = currentState;
}
