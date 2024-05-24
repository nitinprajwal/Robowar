#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AccelStepper.h>

// Wi-Fi credentials
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Define pin connections for the stepper motor
const int dirPin = 25;  // Direction pin
const int stepPin = 26; // Step pin

// Define motor interface type
#define motorInterfaceType 1

// Create an instance of the AccelStepper class
AccelStepper myStepper(motorInterfaceType, stepPin, dirPin);

// Create an instance of the AsyncWebServer class
AsyncWebServer server(80);

// HTML and JavaScript for the web interface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Stepper Motor Control</title>
  <style>
    body {font-family: Arial; display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; margin: 0;}
    h2 {font-size: 2.0rem;}
    button {padding: 10px 20px; margin: 10px; font-size: 1.5rem;}
  </style>
</head>
<body>
  <h2>Stepper Motor Control</h2>
  <button onclick="moveMotor('forward')">Move Forward</button>
  <button onclick="moveMotor('backward')">Move Backward</button>
  <script>
    function moveMotor(direction) {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/move?direction=" + direction, true);
      xhr.send();
    }
  </script>
</body>
</html>
)rawliteral";

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Set motor parameters for smooth movement
  myStepper.setMaxSpeed(700);      // Maximum speed (steps per second)
  myStepper.setAcceleration(300);  // Acceleration (steps per second squared)
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Set up the web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/move", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("direction")) {
      String direction = request->getParam("direction")->value();
      if (direction == "forward") {
        myStepper.moveTo(myStepper.currentPosition() + 1000);  // Move forward
      } else if (direction == "backward") {
        myStepper.moveTo(myStepper.currentPosition() - 1000);  // Move backward
      }
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing direction parameter");
    }
  });

  // Start the server
  server.begin();
}

void loop() {
  // Move the motor
  myStepper.run();
}
