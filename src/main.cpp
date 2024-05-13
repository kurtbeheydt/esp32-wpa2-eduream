#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>

#include "certificate.h"
#include "esp_wpa2.h"
#include "secrets.h"

WiFiClientSecure client;
const char *ssid = EAP_SSID;
const char *test_root_ca = EAP_ROOT_CERTIFICATE;

int counter = 0;

void connectToWpa2Enterprise() {
    Serial.println();
    Serial.print("Connecting to network: ");
    Serial.println(ssid);
    WiFi.disconnect(true);                                                                                  // disconnect form wifi to set new wifi connection
    WiFi.mode(WIFI_STA);                                                                                    // init wifi mode
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ANONYMOUS_IDENTITY, strlen(EAP_ANONYMOUS_IDENTITY));  // provide identity
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));                      // provide username
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));                      // provide password
    esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT();
    esp_wifi_sta_wpa2_ent_enable(&config);
    WiFi.begin(ssid);  // connect to wifi
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        counter++;
        if (counter >= 60) {  // after 30 seconds timeout - reset board (on unsucessful connection)
            Serial.print("restart\n");
            ESP.restart();
        }
    }
    Serial.print("setting ca cert");
    client.setCACert(test_root_ca);
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address set: ");
    Serial.println(WiFi.localIP());  // print LAN IP
}

void checkWpa2EnterpriseConnection() {
    if (WiFi.status() == WL_CONNECTED) {         // if we are connected to wifi network
        counter = 0;                             // reset counter
    } else if (WiFi.status() != WL_CONNECTED) {  // if we lost connection, retry
        WiFi.begin(ssid);
    }
    while (WiFi.status() != WL_CONNECTED) {  // during lost connection, print dots
        delay(500);
        Serial.print(".");
        counter++;
        if (counter >= 60) {  // 30 seconds timeout - reset board
            ESP.restart();
        }
    }
}

void httpGet(char *uri) {
    HTTPClient http;

    http.begin(uri);
    int httpCode = http.GET();

    if (httpCode > 0) {  // Check for the returning code
        delay(1000);     // Wait for 1000 millisecond(s)
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
    } else {
        Serial.println("Error on HTTP request");
    }

    http.end();  // Free the resources
}

void setup() {
    Serial.begin(115200);
    delay(10);
    connectToWpa2Enterprise();
}

void loop() {
    checkWpa2EnterpriseConnection();

    httpGet("http://jsonplaceholder.typicode.com/comments?id=10");
    
    delay(10000);
}
