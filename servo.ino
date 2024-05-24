#include <WiFi.h>
#include <Servo.h>

Servo steeringServo;  // create servo object to control the steering

// GPIO the servo is attached to
static const int servoPin = 13;

// Replace with your network credentials
const char* ssid     = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Decode HTTP GET value
String valueString = String(5);
int pos1 = 0;
int pos2 = 0;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);

  steeringServo.attach(servoPin);  // attaches the servo on the servoPin to the servo object
  steeringServo.write(90);         // sets the servo to the initial position of 90 degrees (neutral position)

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>body { text-align: left; font-family: \"Trebuchet MS\", Arial; margin-left: 20px; display: flex; flex-direction: column; justify-content: center; height: 100vh;}");
            client.println(".slider { width: 300px; height: 20px; -webkit-appearance: none; appearance: none; background: #ddd; outline: none; opacity: 0.7; transition: opacity .2s;}");
            client.println(".slider::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 30px; height: 30px; background: #4CAF50; cursor: pointer;}");
            client.println(".slider::-moz-range-thumb { width: 30px; height: 30px; background: #4CAF50; cursor: pointer;}");
            client.println(".slider-container { margin-top: 10px; }");
            client.println(".degree-display { font-size: 24px; margin-bottom: 10px; }</style>");
            client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>");
            client.println("</head><body><div class=\"degree-display\" id=\"degreeDisplay\">Rotation: 0°</div>");
            client.println("<div class=\"slider-container\"><input type=\"range\" min=\"0\" max=\"180\" class=\"slider\" id=\"servoSlider\" value=\"90\"/></div>");
            client.println("<script>var slider = document.getElementById(\"servoSlider\"); var degreeDisplay = document.getElementById(\"degreeDisplay\");");
            client.println("slider.oninput = function() { var value = this.value; var rotation = value - 90; degreeDisplay.textContent = \"Rotation: \" + rotation + \"°\"; $.get(\"/?value=\" + value); };");
            client.println("slider.onmouseup = function() { this.value = 90; degreeDisplay.textContent = \"Rotation: 0°\"; $.get(\"/?value=90\"); };");
            client.println("slider.ontouchend = function() { this.value = 90; degreeDisplay.textContent = \"Rotation: 0°\"; $.get(\"/?value=90\"); };</script>");
            client.println("</body></html>");

            // GET /?value=180& HTTP/1.1
            if(header.indexOf("GET /?value=") >= 0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1 + 1, pos2);

              // Rotate the servo
              steeringServo.write(valueString.toInt());
              Serial.println(valueString);
            }         
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
