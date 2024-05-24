#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>

// Replace with your network credentials
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Create servo objects for steering and ESC
Servo steeringServo;
Servo ESC;

// GPIO pins
const int servoPin = 13; // Steering servo pin
const int signalOut = 15; // ESC signal pin

// Web server setup
AsyncWebServer server(80);
String steeringValueString = "90"; // default to neutral position for steering
String escValueString = "0"; // default to zero for ESC

const int minPWM = 1000;
const int maxPWM = 2000;

// HTML and JavaScript for the web interface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Car Control</title>
  <style>
    body {font-family: Arial; display: flex; flex-direction: column; align-items: flex-start; justify-content: center; height: 100vh; margin: 0; padding-left: 10px;}
    h2 {font-size: 2.0rem;}
    .slider { -webkit-appearance: none; width: 300px; height: 25px; background: #ddd; outline: none; opacity: 0.7; transition: opacity .2s;}
    .slider::-webkit-slider-thumb { -webkit-appearance: none; width: 30px; height: 30px; background: #4CAF50; cursor: pointer;}
    .slider::-moz-range-thumb { width: 30px; height: 30px; background: #4CAF50; cursor: pointer; }
    .slider-container { margin-top: 10px; }
    .degree-display { font-size: 24px; margin-bottom: 10px; }
  </style>
</head>
<body>
  <h2>ESC Control</h2>
  <p><span id="escValue">0</span></p>
  <div class="slider-container">
    <input type="range" min="0" max="180" class="slider" id="escSlider" value="0"/>
  </div>
  <h2>Steering Control</h2>
  <div class="degree-display" id="degreeDisplay">Rotation: 0°</div>
  <div class="slider-container">
    <input type="range" min="0" max="180" class="slider" id="servoSlider" value="90"/>
  </div>
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
  <script>
    // Steering control
    var servoSlider = document.getElementById("servoSlider");
    var degreeDisplay = document.getElementById("degreeDisplay");
    servoSlider.oninput = function() {
      var value = this.value;
      var rotation = value - 90;
      degreeDisplay.textContent = "Rotation: " + rotation + "°";
      $.get("/steering?value=" + value);
    };
    servoSlider.onmouseup = function() {
      this.value = 90;
      degreeDisplay.textContent = "Rotation: 0°";
      $.get("/steering?value=90");
    };
    servoSlider.ontouchend = function() {
      this.value = 90;
      degreeDisplay.textContent = "Rotation: 0°";
      $.get("/steering?value=90");
    };
    // ESC control
    var escSlider = document.getElementById("escSlider");
    var escValue = document.getElementById("escValue");
    escSlider.oninput = function() {
      var value = this.value;
      escValue.textContent = value;
      $.get("/esc?value=" + value);
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
      steeringServo.write(value);
      Serial.println("Steering: " + steeringValueString);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/esc", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      escValueString = request->getParam("value")->value();
      int value = escValueString.toInt();
      ESC.write(value);
      Serial.println("ESC: " + escValueString);
    }
    request->send(200, "text/plain", "OK");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // No need to handle clients manually as AsyncWebServer handles this
}
