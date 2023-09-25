/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Replace with your network credentials
const char* ssid     = "fkrwhvio";
const char* password = "vrjnkrenk";
#define FIREBASE_HOST "fyphealthcheck-default-rtdb.firebaseio.com/"
#define FIREBASE_Authorization_key "vIK2Xjlzgqgjk7ZKMvnbGPNzuRSlkdkFZsy3JFkX"

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to stoe the current output state
String output5State = "7";
String output4State = "100";
String output3State = "28";

FirebaseData firebaseData;
FirebaseJson json;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);

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
  Firebase.begin(FIREBASE_HOST,FIREBASE_Authorization_key);
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
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
            int flowRate = 0;
            int ph = random(6,8);
            int temp = random(25,30);
            Serial.print("Flow rate: ");
            Serial.print(flowRate);
            Serial.print("pH Level: ");
            Serial.print(ph);
            Serial.print("Water temperature: ");
            Serial.print(temp);
            // turns the GPIOs on and off
            if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("Water flow A:");
              output5State = flowRate;
              Serial.print("Flow rate: ");
              Serial.print(output5State);
            } 
            else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("Ph Value");
              output4State = ph;
              Serial.print(output4State);
            }
            else if (header.indexOf("GET /3/on") >= 0) {
              Serial.println("Temperature:");
              output3State = temp;
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Water Management System</h1>");
            // If the output5State is off, it displays the ON button       
            client.println("<p><a href=\"/5/on\"><button class=\"button\">Water Summary : </button></a></p>");
            client.println("<p> 5 - Water flow (in L/min) : " + output5State + "</p>");
            client.println("<p><a href=\"/4/on\"><button class=\"button button2\">Ph Level : </button></a></p>");
            client.println("<p> 4 - pH value: " + output4State + "</p>");
            client.println("<p><a href=\"/3/on\"><button class=\"button button3\">Temperature : </button></a></p>");
            client.println("<p> 3 - water temp (. c): " + output3State + "</p>");
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            Firebase.setInt(firebaseData, "/ESP8266_APP/FLOW", flowRate);
            Firebase.setInt(firebaseData, "/ESP8266_APP/PH", ph);
            Firebase.setInt(firebaseData, "/ESP8266_APP/TEMP", temp);
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
