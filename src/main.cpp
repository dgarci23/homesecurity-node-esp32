#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <EEPROM.h>

#define BOARD_ID 1
#define WIFI_SSID "ND-guest" 

#define MAGNET 0
#define MOVEMENT 1

//MAC Address of the receiver 
uint8_t broadcastAddress[6];
uint8_t board_id;
String ssid;

// Performance measurements
int t_total;
int t_eeprom;
int t_channel_sel;
int t_espnow;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    int id;
    int value;
} struct_message;

//Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

int32_t getWiFiChannel() {
  if (int32_t n = WiFi.scanNetworks()) {
    for (uint8_t i=0; i<n; i++) {
        if (ssid == WiFi.SSID(i)) {
            return WiFi.channel(i);
        }
    }
  }
  return 0;
}

void printPerformance() {
  Serial.printf("------ PERFORMANCE RESULTS ------\n");
  Serial.printf("Total Latency: %d\n", t_total);
  Serial.printf("   EEPROM Latency: %d\n", t_eeprom);
  Serial.printf("   Channel Selection Latency: %d\n", t_channel_sel);
  Serial.printf("   ESP NOW Latency: %d\n", t_espnow);
  Serial.printf("---------------------------------\n");
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, MOVEMENT);
  Serial.println("Going to sleep now");
  t_total = millis() - t_total;
  t_espnow = millis() - t_espnow;
  printPerformance();
  esp_deep_sleep_start();
}

void getConfig() {
  for (int i = 0; i < 6; i++){
    broadcastAddress[i] = EEPROM.read(i);
  }
  board_id = EEPROM.read(6);
  ssid = EEPROM.readString(7);
}

void saveInitialConfig() {
  // MAC Address
  EEPROM.write(0, 0x8C);
  EEPROM.write(1, 0x4B);
  EEPROM.write(2, 0x14);
  EEPROM.write(3, 0x9F);
  EEPROM.write(4, 0x03);
  EEPROM.write(5, 0xD4);
  // ID
  EEPROM.write(6, BOARD_ID);
  // SSID
  EEPROM.writeString(7, WIFI_SSID);
  EEPROM.commit();
  Serial.println("Configuration data persisted.");
}



void setup() {
  t_total = millis();

  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  //Init Serial Monitor
  Serial.begin(115200);

  EEPROM.begin(40);

  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_EXT0){
    saveInitialConfig();
  }

  t_eeprom = millis();
  getConfig();
  t_eeprom = millis() - t_eeprom;

  // Set device as a Wi-Fi Station and set channel
  WiFi.mode(WIFI_STA);

  t_channel_sel = millis();
  int32_t channel = getWiFiChannel();
  t_channel_sel = millis() - t_channel_sel;

  //WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  //WiFi.printDiag(Serial); // Uncomment to verify channel change after

  t_espnow = millis();
  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  //Register peer
  //esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;
  
  //Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  myData.id = board_id;
  myData.value = random(0,50);
     
  //Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
    
  }
  else {
    Serial.println("Error sending the data");
  }
} 
void loop() {
}