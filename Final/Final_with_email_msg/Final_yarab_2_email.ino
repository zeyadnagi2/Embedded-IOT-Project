#include <EMailSender.h>
#include <ESP32Servo.h>
#include <Keypad.h>
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "ESP32_MailClient.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <EmailFunction.h>


// To send Emails using Gmail on port 465 (SSL)
#define emailSenderAccount    "iot67149@gmail.com"
#define emailSenderPassword   "xrstglxjwdobixnb"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "[ALERT] Someone Enter the password more than 3 times"
#define inputMessage          "badr0ahmed5@gmail.com"
#define enableEmailChecked    "checked"
// Default Recipient Email Address


Servo Myservo1;
Servo Myservo2;

#define DO_PIN_FLAME 14
#define SENSOR_PIN_OBSTACLE 33
#define DO_PIN_LIGHT 35
#define RAIN_SENSOR_PIN 34
#define SERVO_PIN_1 26
#define SERVO_PIN_2 12
#define NUM_RELAYS 3
#define RELAY_NO true

#define MOTION_SENSOR_PIN 23  

#define ROW_NUM 4
#define COLUMN_NUM 4
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte pin_rows[ROW_NUM] = {19, 18, 5, 17};
byte pin_column[COLUMN_NUM] = {16, 4, 0, 2};
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);
const String password = "7890";
String input_password;

int relayGPIOs[NUM_RELAYS] = {32, 13, 15};

#define DHT_SENSOR_PIN 27
#define DHT_SENSOR_TYPE DHT11
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);


int lastState = HIGH;  
int currentState;      

bool doorLocked = true; 

int wrongPasswordAttempts = 0;
bool systemStolen = false;

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 1.5rem; color: #333;} /* Adjusted font size to 1.5rem */
    body {max-width: 800px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>HOME AUTOMATION</h2>
  <p id="flameStatus">Flame Status: <span id="flameValue"></span></p>
  <p id="obstacleStatus">Obstacle Status: <span id="obstacleValue"></span></p>
  <p id="lightStatus">Light Status: <span id="lightValue"></span></p>
  <p id="rainStatus">Rain Status: <span id="rainValue"></span></p>
  <p id="temperature">Temperature: <span id="temperatureValue"></span></p>
  <p id="doorStatus">Door Status: <span id="doorValue"></span></p>
  <p id="securityStatus">Security Status: <span id="securityValue"></span></p> <!-- Added security status display -->
  %BUTTONPLACEHOLDER%
  <script>
    function toggleCheckbox(element) {
      var xhr = new XMLHttpRequest();
      if (element.checked) { xhr.open("GET", "/update?relay=" + element.id + "&state=1", true); }
      else { xhr.open("GET", "/update?relay=" + element.id + "&state=0", true); }
      xhr.send();
    }
    
    function updateSensorStatuses() {
      fetch('/sensor-status')
        .then(response => response.json())
        .then(data => {
          document.getElementById('flameValue').innerText = data.flameStatus;
          document.getElementById('obstacleValue').innerText = data.obstacleStatus;
          document.getElementById('lightValue').innerText = data.lightStatus;
          document.getElementById('rainValue').innerText = data.rainStatus;
          document.getElementById('temperatureValue').innerText = data.temperature;
          document.getElementById('doorValue').innerText = data.doorStatus;
          document.getElementById('securityValue').innerText = data.securityStatus; // Update security status
        });
    }

    setInterval(updateSensorStatuses, 1000);
  </script>
</body>
</html>
)rawliteral";



String relayState(int numRelay){
  if (RELAY_NO) {
    return digitalRead(relayGPIOs[numRelay - 1]) ? "" : "checked";
  } else {
    return digitalRead(relayGPIOs[numRelay - 1]) ? "checked" : "";
  }
}

String processor(const String& var){
  if (var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    for (int i = 1; i <= NUM_RELAYS; i++){
      buttons+= "<h4>ROOM : " + String(i) +  "</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" + String(i) + "\" "+ relayState(i) +"><span class=\"slider\"></span></label>";
    }
    return buttons;
  }
  return String();
}


void updateDoorLockStatus() {
  String doorStatusJson = "{\"doorLocked\":\"" + String(doorLocked ? "Locked" : "Unlocked") + "\"}";
  server.on("/door-status", HTTP_GET, [doorStatusJson](AsyncWebServerRequest *request){
    request->send(200, "application/json", doorStatusJson);
  });
}

void updateSecurityStatus() {
  String securityStatusJson = "{\"securityStatus\":\"" + String(systemStolen ? "Stolen" : "Normal") + "\"}";
  server.on("/security-status", HTTP_GET, [securityStatusJson](AsyncWebServerRequest *request){
    request->send(200, "application/json", securityStatusJson);
  });
}

static unsigned long lastKeypadCheckTime = 0;
const unsigned long KEYPAD_CHECK_INTERVAL = 0;  

bool emailSend = false;

void handleKeypadInput() {
  if (systemStolen) {
    SendEmailNow(emailSenderAccount,emailSenderPassword,inputMessage,smtpServer,smtpServerPort,emailSubject);
    
  }
  unsigned long currentTime = millis();

  if (currentTime - lastKeypadCheckTime >= KEYPAD_CHECK_INTERVAL) {
    char key = keypad.getKey();
    if (key) {
      Serial.println(key);
      if (key == '*') {
        input_password = "";
      } else if (key == '#') {
        if (password == input_password) {
          Serial.println("The password is correct, ACCESS GRANTED!");

          wrongPasswordAttempts = 0;

          doorLocked = !doorLocked; 

          updateDoorLockStatus();

          if (doorLocked) {
            Myservo2.write(0);  
          } else {
            Myservo2.write(60); 
            delay(5000);
            Myservo2.write(0);
            doorLocked = true;
            updateDoorLockStatus();
          }
        } else {
          Serial.println("The password is incorrect, ACCESS DENIED!");

          wrongPasswordAttempts++;

          if (wrongPasswordAttempts >= 3) {
            Serial.println("Too many wrong attempts. System marked as stolen!");
            systemStolen = true;
            delay(3000);
            systemStolen = false;
          }
        }

        input_password = "";
      } else {
        input_password += key;
      }
    }
    lastKeypadCheckTime = currentTime;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(DO_PIN_FLAME, INPUT);
  pinMode(SENSOR_PIN_OBSTACLE, INPUT);
  pinMode(DO_PIN_LIGHT, INPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);

  Myservo1.attach(SERVO_PIN_1);
  Myservo2.attach(SERVO_PIN_2);
  Myservo1.write(0);
  Myservo2.write(0);

  WiFi.begin("Zoz", "123456789");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  for (int i = 0; i < NUM_RELAYS; i++) {
    pinMode(relayGPIOs[i], OUTPUT);
    digitalWrite(relayGPIOs[i], RELAY_NO ? HIGH : LOW);
  }

  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("relay") && request->hasParam("state")) {
      int relayNum = request->getParam("relay")->value().toInt();
      int relayState = request->getParam("state")->value().toInt();

      if (RELAY_NO) {
        relayState = !relayState;
      }

      digitalWrite(relayGPIOs[relayNum - 1], relayState);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/sensor-status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String sensorStatusJson = "{";
    sensorStatusJson += "\"flameStatus\":\"" + String((digitalRead(DO_PIN_FLAME) == HIGH ? "Detected" : "Not Detected")) + "\",";
    sensorStatusJson += "\"obstacleStatus\":\"" + String((digitalRead(SENSOR_PIN_OBSTACLE) == HIGH ? "Not Present" : "Present")) + "\",";
    sensorStatusJson += "\"lightStatus\":\"" + String((digitalRead(DO_PIN_LIGHT) == LOW ? "Light" : "Dark")) + "\",";
    sensorStatusJson += "\"rainStatus\":\"" + String((digitalRead(RAIN_SENSOR_PIN) == HIGH ? "No Rain" : "Rain Detected")) + "\",";

    float humi = dht_sensor.readHumidity();
    float tempC = dht_sensor.readTemperature();
    float tempF = dht_sensor.readTemperature(true);

    if (!isnan(tempC) && !isnan(tempF) && !isnan(humi)) {
      sensorStatusJson += "\"temperature\":\"" + String(tempC) + "°C ~ " + String(tempF) + "°F\"";
    }

    sensorStatusJson += ",\"doorStatus\":\"" + String((doorLocked ? "Locked" : "Unlocked")) + "\"";
    sensorStatusJson += ",\"securityStatus\":\"" + String((systemStolen ? "Stealing" : "Safe")) + "\"";
    sensorStatusJson += "}";
    request->send(200, "application/json", sensorStatusJson);
  });

  server.on("/door-status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String doorStatusJson = "{\"doorLocked\":\"" + String(doorLocked ? "Locked" : "Unlocked") + "\"}";
    request->send(200, "application/json", doorStatusJson);
  });

  server.begin();
}

void loop() {
  int flame_state = digitalRead(DO_PIN_FLAME);
  if (flame_state == HIGH) {
      
    // Handle flame detection logic if needed
  }

  currentState = digitalRead(MOTION_SENSOR_PIN);
  if (lastState == HIGH && currentState == LOW) {
    // Motion detected logic
  } else if (lastState == LOW && currentState == HIGH) {
    // Motion cleared logic
  }

  handleKeypadInput();

  int lightState = digitalRead(DO_PIN_LIGHT);
  if (lightState == LOW) {
    char keyPressed = keypad.getKey();
    if (keyPressed == 'A') {
      for (int i = 0; i < NUM_RELAYS; i++) {
      digitalWrite(relayGPIOs[i], RELAY_NO ? HIGH : LOW);
    }
   }
  } else {
    for (int i = 0; i < NUM_RELAYS; i++) {
      digitalWrite(relayGPIOs[i], RELAY_NO ? LOW : HIGH);
    }
  }

  int rainState = digitalRead(RAIN_SENSOR_PIN);
  static int prevRainState = LOW;
  if (rainState != prevRainState) {
    prevRainState = rainState;
    if (rainState == HIGH) {
      Serial.println("No Rain");
    } else {
      Serial.println("Rain Detected"); 
    }
  }

  float humi = dht_sensor.readHumidity();
  float tempC = dht_sensor.readTemperature();
  float tempF = dht_sensor.readTemperature(true);

  if (tempC > 30.0) {
    Serial.println("Temperature is greater than 30°C. Turn on fans!");
    Myservo1.write(0);
  } else {
    Myservo1.write(90);
  }
}
