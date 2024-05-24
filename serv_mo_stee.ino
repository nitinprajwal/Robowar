#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>
#include <AccelStepper.h>

// Replace with your network credentials
const char* ssid = "replace_with_your_ssid";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Create servo objects for steering and ESC
Servo steeringServo;
Servo ESC;

// GPIO pins for servos
const int servoPin = 13; // Steering servo pin
const int signalOut = 15; // ESC signal pin

// Define pin connections for stepper motor
const int dirPin = 25;  // Stepper motor direction pin
const int stepPin = 26; // Stepper motor step pin

// Define motor interface type
#define motorInterfaceType 1

// Create an instance of AccelStepper
AccelStepper myStepper(motorInterfaceType, stepPin, dirPin);

// Web server setup
AsyncWebServer server(80);
String steeringValueString = "90"; // Default to neutral position for steering
String escValueString = "0"; // Default to zero for ESC
String stepperValueString = "0"; // Default to zero for stepper motor speed

const int minPWM = 1000;
const int maxPWM = 2000;

// HTML and JavaScript for the web interface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Car Control</title>
  <style>
    body {
      font-family: Arial;
      display: flex;
      justify-content: space-between;
      height: 100vh;
      margin: 0;
      padding: 10px;
      overflow: hidden;
    }
    h2 {
      font-size: 2rem;
    }
    .slider {
      -webkit-appearance: none;
      width: 300px;
      height: 25px;
      background: #ddd;
      outline: none;
      opacity: 0.7;
      transition: opacity 0.2s;
    }
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 30px;
      height: 30px;
      background: #4caf50;
      cursor: pointer;
    }
    .slider::-moz-range-thumb {
      width: 30px;
      height: 30px;
      background: #4caf50;
      cursor: pointer;
    }
    .slider-container {
      margin-top: 10px;
    }
    .degree-display {
      font-size: 24px;
      margin-bottom: 10px;
    }
    .vertical-slider {
      -webkit-appearance: none;
      width: 25px;
      height: 200px;
      background: #ddd;
      outline: none;
      opacity: 0.7;
      transition: opacity 0.2s;
    }
    .vertical-slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 30px;
      height: 30px;
      background: #4caf50;
      cursor: pointer;
    }
    .vertical-slider::-moz-range-thumb {
      width: 30px;
      height: 30px;
      background: #4caf50;
      cursor: pointer;
    }
    .column {
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .stepper-section {
      margin-top: 180px;
      height: 250px;
      display: block; /* Adjust this value to move the section higher or lower */
    }
  </style>
</head>
<body>
  <div class="column">
    <h2>ESC Control</h2>
    <p><span id="escValue">0</span></p>
    <div class="slider-container">
      <input
        type="range"
        min="0"
        max="180"
        class="slider"
        id="escSlider"
        value="0"
      />
    </div>
    <h2>Steering Control</h2>
    <div class="degree-display" id="degreeDisplay">Rotation: 0°</div>
    <div class="slider-container">
      <input
        type="range"
        min="10"
        max="170"
        class="slider"
        id="servoSlider"
        value="90"
      />
    </div>
  </div>
  <div class="stepper-section">
    <div class="slider-container">
      <input
        type="range"
        min="-700"
        max="700"
        value="0"
        class="slider"
        id="stepperSlider"
        orient="vertical"
        style="transform: rotate(90deg); direction: rtl;"
      />
    </div>
  </div>
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
  <script>
    // Steering control
    var servoSlider = document.getElementById("servoSlider");
    var degreeDisplay = document.getElementById("degreeDisplay");
    servoSlider.oninput = function () {
      var value = this.value;
      var rotation = value - 90;
      degreeDisplay.textContent = "Rotation: " + rotation + "°";
      $.get("/steering?value=" + value);
    };
    servoSlider.onmouseup = function () {
      this.value = 90;
      degreeDisplay.textContent = "Rotation: 0°";
      $.get("/steering?value=90");
    };
    servoSlider.ontouchend = function () {
      this.value = 90;
      degreeDisplay.textContent = "Rotation: 0°";
      $.get("/steering?value=90");
    };
    // ESC control
    var escSlider = document.getElementById("escSlider");
    var escValue = document.getElementById("escValue");
    escSlider.oninput = function () {
      var value = this.value;
      escValue.textContent = value;
      $.get("/esc?value=" + value);
    };
    // Stepper motor control
    var stepperSlider = document.getElementById("stepperSlider");
    var stepperSpeed = document.getElementById("stepperSpeed");
    stepperSlider.oninput = function () {
      var value = this.value;
      stepperSpeed.textContent = value;
      $.get("/stepper?value=" + value);
    };
    stepperSlider.onmouseup = function () {
      this.value = 0;
      stepperSpeed.textContent = 0;
      $.get("/stepper?value=0");
    };
    stepperSlider.ontouchend = function () {
      this.value = 0;
      stepperSpeed.textContent = 0;
      $.get("/stepper?value=0");
    };
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  // Initialize servos
  steeringServo.attach(servoPin);
  steeringServo.write(90); // Neutral position
  ESC.attach(signalOut, minPWM, maxPWM);

  // Initialize stepper motor
  myStepper.setMaxSpeed(700);      // Adjust maximum speed (steps per second)
  myStepper.setAcceleration(300);  // Adjust acceleration (steps per second squared)

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Set up web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/steering", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      steeringValueString = request->getParam("value")->value();
      int value = steeringValueString.toInt();
      // Limit value to 10-170 degrees
      value = constrain(value, 10, 170);
      steeringServo.write(value);
      Serial.println("Steering: " + steeringValueString);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/esc", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      escValueString = request->getParam("value")->value();
      int value = escValueString.toInt();
      // Map value from 0-180 to minPWM-maxPWM
      int pwmValue = map(value, 0, 180, minPWM, maxPWM);
      ESC.writeMicroseconds(pwmValue);
      Serial.println("ESC: " + escValueString + " -> PWM: " + pwmValue);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/stepper", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      stepperValueString = request->getParam("value")->value();
      int value = stepperValueString.toInt();
      myStepper.setSpeed(value);
      myStepper.runSpeed();
      Serial.println("Stepper Speed: " + stepperValueString);
    }
    request->send(200, "text/plain", "OK");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // No need to handle clients manually as AsyncWebServer handles this
}
