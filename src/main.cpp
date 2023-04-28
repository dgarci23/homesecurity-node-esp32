#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <initial.h>

#define BOARD_ID 1
#define WIFI_SSID "ND-guest" 

#define MAGNET 0
#define MOVEMENT 1

#define TRIGGER 0
#define BATTERY 1
#define LOW_BATTERY 2

//MAC Address of the receiver 
uint8_t broadcastAddress[6];
uint8_t board_id = 1;
String ssid;

// Battery Status
RTC_DATA_ATTR bool battery_status = true;

// Performance measurements
int t_total;
int t_eeprom;
int t_channel_sel;
int t_espnow;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    int id;
    int battery;
} struct_message;

//Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

void printPerformance() {
  Serial.printf("------ PERFORMANCE RESULTS ------\n");
  Serial.printf("Total Latency: %d\n", t_total);
  Serial.printf("   ESP NOW Latency: %d\n", t_espnow);
  Serial.printf("---------------------------------\n");
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  esp_deep_sleep_enable_gpio_wakeup(1ULL << 4, ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_sleep_enable_timer_wakeup(5*1000000);
  Serial.println("Going to sleep now");
  t_total = millis() - t_total;
  t_espnow = millis() - t_espnow;
  printPerformance();
  esp_deep_sleep_start();
}

void sendStatus() {
  
  // Set device as a Wi-Fi Station and set channel
  WiFi.mode(WIFI_STA);

  t_espnow = millis();
  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  
  //Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;
  
  //Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
    
  //Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }
}

void setup() {
  t_total = millis();

  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  pinMode(4, INPUT_PULLDOWN);

  Serial.begin(115200);
  
  initSPIFFS();

  readFile();

  if(initMac()) {
    
    getBroadcastAddress(broadcastAddress);
    
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
      float battery_v = (float)analogRead(GPIO_NUM_3)*3.3/4096;
      Serial.printf("Battery Voltage Read: %f\n", battery_v);
      if (battery_v < 1.55 && battery_status) {
        myData.id = board_id;
        myData.battery = 2;
        battery_status = false;
        Serial.printf("Low Battery\n");
        sendStatus();
      } else if (battery_v > 1.55 && !battery_status) {
        myData.id = board_id;
        myData.battery = 1;
        battery_status = true;
        Serial.printf("High Battery\n");
        sendStatus();
      } else {
        esp_deep_sleep_enable_gpio_wakeup(1ULL << 4, ESP_GPIO_WAKEUP_GPIO_LOW);
        esp_sleep_enable_timer_wakeup(5*1000000);
        esp_deep_sleep_start();
      }
      return;
    }

    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO) {
      Serial.printf("Sensor Triggered\n");
      myData.id = board_id;
      myData.battery = 0;
      sendStatus();
      return;
    }
    esp_deep_sleep_enable_gpio_wakeup(1ULL << 4, ESP_GPIO_WAKEUP_GPIO_LOW);
    esp_sleep_enable_timer_wakeup(5*1000000);
    esp_deep_sleep_start();
  }
  else {
    initialConfig();
  }
  

  return;
} 
void loop() {
}