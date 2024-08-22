#include <Arduino.h>

// Define the GPIO pin connected to the rain sensor.
int RAIN_SENSOR_PIN = 33; // Change this to the actual pin you are using

// Variable to store the previous state of the rain sensor.
int prevRainState = LOW;

void setup() {
  // Initialize the serial port.
  Serial.begin(115200);

  // Set the rain sensor pin as an input.
  pinMode(RAIN_SENSOR_PIN, INPUT);
}

void loop() {
  // Read the value of the rain sensor pin.
  int rainState = digitalRead(RAIN_SENSOR_PIN);

  // Check if the state has changed.
  if (rainState != prevRainState) {
    // Update the previous state.
    prevRainState = rainState;

    // Print the state of the rain sensor.
    if (rainState == HIGH) {
      Serial.println("No Rain");
    } else {
      Serial.println("Rain Detected");
    }
  }

  // You can add a delay between readings if needed.
  delay(1000);
}
