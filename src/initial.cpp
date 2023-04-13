#include <initial.h>


IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "mac";

// File paths to save input values permanently
const char* macPath = "/mac.txt";

String mac;

void getBroadcastAddress(uint8_t broadcastAddress[]) {
  Serial.println(mac);
  sscanf(mac.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x",
           &broadcastAddress[0], &broadcastAddress[1], &broadcastAddress[2], &broadcastAddress[3], &broadcastAddress[4], &broadcastAddress[5]);
}

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

// Initialize WiFi
bool initMac() {
  if(mac==""){
    Serial.printf("Undefined MAC.\n");
    return false;
  }

  return true;
}

void readFile() {
    mac = readFile(SPIFFS, macPath);
}

// Initial Config
void initialConfig() {

    // Setting Soft AP
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);
    IPAddress IP = WiFi.softAPIP();

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/wifimanager.html", "text/html");
    });
    
    server.serveStatic("/", SPIFFS, "/");
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            mac = p->value().c_str();
            Serial.printf("SSID set to: %s\n", mac.c_str());
            writeFile(SPIFFS, macPath, mac.c_str());
          }
        }
      }
      request->send(200, "text/plain", "Home Security Sensor configured.");
      delay(3000);
      ESP.restart();
    });
    server.begin();
}