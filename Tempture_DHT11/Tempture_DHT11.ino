#include <DHT.h>
#include <ESP32Servo.h>

#define DHT_SENSOR_PIN  27 // ESP32 pin GPIO21 connected to DHT11 sensor
#define DHT_SENSOR_TYPE DHT11
#define SERVO_PIN       26  // Choose an appropriate pin for your servo

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
Servo servo;

void setup() {
  Serial.begin(115200);
  dht_sensor.begin(); // initialize the DHT sensor
  servo.attach(SERVO_PIN); // attach the servo to the specified pin
  servo.write(0); // set the initial position of the servo (0 degrees)
}

void loop() {
  // read humidity
  float humi  = dht_sensor.readHumidity();
  // read temperature in Celsius
  float tempC = dht_sensor.readTemperature();
  // read temperature in Fahrenheit
  float tempF = dht_sensor.readTemperature(true);

  // check whether the reading is successful or not
  if (isnan(tempC) || isnan(tempF) || isnan(humi)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(humi);
    Serial.print("%");

    Serial.print("  |  ");

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.print("°C  ~  ");
    Serial.print(tempF);
    Serial.println("°F");

    // Check if temperature is greater than 30°C
    if (tempC > 30.0) {
      Serial.println("Temperature is greater than 30°C. Turn on fans!");
      // Turn on the servo
      servo.write(0); // You may need to adjust this angle based on your servo characteristics
    } else {
      // Turn off the servo
      servo.write(90);
    }
  }

  // wait 2 seconds between readings
  delay(2000);
}
