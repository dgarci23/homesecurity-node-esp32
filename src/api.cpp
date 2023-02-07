#include <api.h> 

//Your Domain name with URL path or IP address with path
String serverName = "https://aapqa4qfkg.execute-api.us-east-1.amazonaws.com/dev";

void configureWiFi(const char* ssid) {
    WiFi.begin(ssid);
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
}

String connectApi(String path, const char* method) {
    String payload;
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      String serverPath = serverName + path;
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
      
      // Send HTTP GET request
      int httpResponseCode = http.sendRequest(method,"");
      //int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        //Serial.print("HTTP Response code: ");
        //Serial.println(httpResponseCode);
        payload = http.getString();
        //Serial.println(payload);
      }
      else {
        //Serial.print("Error code: ");
        //Serial.println(httpResponseCode);
        payload = "Error";
      }
      // Free resources
      http.end();
    }
    else {
      //Serial.println("WiFi Disconnected");
        payload = "WiFi Disconnected";
    }
    return payload;
}

String getUser(String userId){
    return connectApi("/user/"+userId, "GET");
}

String getSensors(String userId){
    return connectApi("/sensor/"+userId, "GET");
}

void updateSensor(String userId, String sensorId, String sensorStatus){
    connectApi("/sensor/"+userId+"?sensorId="+sensorId+"&sensorStatus="+sensorStatus, "PUT");
}