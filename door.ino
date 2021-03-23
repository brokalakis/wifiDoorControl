/*********
 * 
 * Konstantinos Brokalakis
 * Door
 * for MOF ;-) 
   
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <Servo.h>


// Replace with your network credentials
#ifndef STASSID
#define STASSID "your_wifi_id"
#define STAPSK  "your_wifi_pass"
#endif


Servo myservo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position
const char* ssid = STASSID;
const char* password = STAPSK;
const char* deviceName = "mydoor";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String doorOpenRequest = "close";
String doorState = "closed";
int doorSensorState = 0;
// Assign output variables to GPIO pins

const int doorSensorPin = 4; //D2
const int ledPin = 5; //D1
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  myservo.attach(2);  // this is D4
 
  pinMode(doorSensorPin, INPUT);
  pinMode(ledPin,OUTPUT);
  // Set outputs to LOW
  digitalWrite(ledPin, LOW);


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

void getDoorSensor()
{
  // read the state of the pushbutton value:
  doorSensorState = digitalRead(doorSensorPin);

  // check if the sensor is LOW. If it is, the door is closed:
  if (doorSensorState == LOW) {
    // turn LED off:
    doorState="OPEN";
    
    digitalWrite(ledPin, HIGH);
  } else {
    // turn LED on:
    doorState="CLOSE";
    
    digitalWrite(ledPin, LOW);
  }
}
void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients
  getDoorSensor();
  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
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
            
            // turns the door handle 
            if (header.indexOf("GET /door/open") >= 0) {
              Serial.println("Open door requested");
              doorOpenRequest = "open";
              myservo.write(60);               
            } else if (header.indexOf("GET /door/close") >= 0) {
              Serial.println("Close handle request");
              doorOpenRequest = "close";
              myservo.write(90); 
            } 

            getDoorSensor();
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Door Control</h1>");
            
            // Display current state of door handle  
            client.println("<p>Door handle is " + doorOpenRequest + "</p>");
            // If the output26State is off, it displays the ON button       
            if (doorOpenRequest=="close") {
              client.println("<p><a href=\"/door/open\"><button class=\"button\">Open</button></a></p>");
            } else {
              client.println("<p><a href=\"/door/close\"><button class=\"button button2\">Close</button></a></p>");
            } 

               // Display current state of door handle  
            client.println("<p>The Door is " + doorState + "</p>");
            
            client.println("</body></html>");
            
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
