/*
  Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
  KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
  EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
  OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#include "WiFi.h"
#include "secrets.h"
#include <ArduinoJson.h>
#include <MQTTClient.h>
#include <WiFiClientSecure.h>
#include <Elevator.h>

// The MQTT topics that this device should publish/subscribe
#define DATA_PUBLISH_TOPIC "elevator/esp32/pub"
#define BOOT_PUBLISH_TOPIC "elevator/boot/esp32/pub"
#define BOOT_SUBSCRIBE_TOPIC "elevator/esp32/sub"

#define CURRENT_FLOOR_BRIGHTNESS 255
#define SELECTED_FLOOR_BRIGHTNESS 40

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client(512);

Elevator elevator(4);

#define BUTTON_1 8
#define BUTTON_2 7
#define BUTTON_3 6
#define BUTTON_4 5

#define LED_1 12
#define LED_2 11
#define LED_3 10
#define LED_4 9


void publishMessage() {
    JsonDocument doc;
    doc["device_id"] = "esp32";
    doc["current_floor"] = elevator.getCurrentFloor();
    doc["selected_floor"] = elevator.getSelectedFloor();
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer); // print to client

    client.publish(DATA_PUBLISH_TOPIC, jsonBuffer);
}

void requestInitialConfig() {
    JsonDocument doc;
    doc["topic"] = "elevator/esp32/sub";
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer); // print to client

    client.publish(BOOT_PUBLISH_TOPIC, jsonBuffer);
}

void messageHandler(String &topic, String &payload) {
    Serial.println("incoming: " + topic + " - " + payload);

    JsonDocument doc;
    deserializeJson(doc, payload);
    unsigned int current_floor = doc["current_floor"];
    unsigned int selected_floor = doc["selected_floor"];
    elevator.start(current_floor, selected_floor);
}

void connectAWS() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.println("Connecting to Wi-Fi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // Configure WiFiClientSecure to use the AWS IoT device credentials
    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);

    // Connect to the MQTT broker on the AWS endpoint we defined earlier
    client.begin(AWS_IOT_ENDPOINT, 8883, net);

    // Create a message handler
    client.onMessage(messageHandler);

    Serial.print("Connecting to AWS IOT");

    while (!client.connect(THINGNAME)) {
        Serial.print(".");
        delay(100);
    }

    if (!client.connected()) {
        Serial.println("AWS IoT Timeout!");
        return;
    }

    // Subscribe to a topic
    client.subscribe(BOOT_SUBSCRIBE_TOPIC);

    // Request initial config
    requestInitialConfig();

    Serial.println("AWS IoT Connected!");
}

void setup() {
    Serial.begin(9600);
    connectAWS();

    pinMode(BUTTON_1, INPUT_PULLDOWN);
    pinMode(BUTTON_2, INPUT_PULLDOWN);
    pinMode(BUTTON_3, INPUT_PULLDOWN);
    pinMode(BUTTON_4, INPUT_PULLDOWN);
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(LED_3, OUTPUT);
    pinMode(LED_4, OUTPUT);
}

void loop() {
    if(elevator.update()) {
        publishMessage();
    }

    //delay(1000);
    //Serial.println(digitalRead(1));
    //digitalWrite(2, digitalRead(1));
    //digitalWrite(5, digitalRead(1));

    if ((digitalRead(BUTTON_1) + digitalRead(BUTTON_2) + digitalRead(BUTTON_3) + digitalRead(BUTTON_4)) == 1){
        if (digitalRead(BUTTON_1)) {
            if (elevator.moveToFloor(0)) publishMessage();
        }
        if (digitalRead(BUTTON_2)) {
            if (elevator.moveToFloor(1)) publishMessage();
        }
        if (digitalRead(BUTTON_3)) {
            if (elevator.moveToFloor(2)) publishMessage();
        }
        if (digitalRead(BUTTON_4)) {
            if (elevator.moveToFloor(3)) publishMessage();
        }
    }

    client.loop();

    analogWrite(LED_1, 0);
    analogWrite(LED_2, 0);
    analogWrite(LED_3, 0);
    analogWrite(LED_4, 0);

    switch (elevator.getSelectedFloor()) {
        case 0:
            analogWrite(LED_1, SELECTED_FLOOR_BRIGHTNESS);
            break;
        case 1:
            analogWrite(LED_2, SELECTED_FLOOR_BRIGHTNESS);
            break;
        case 2:
            analogWrite(LED_3, SELECTED_FLOOR_BRIGHTNESS);
            break;
        case 3:
            analogWrite(LED_4, SELECTED_FLOOR_BRIGHTNESS);
            break;
    }

    switch (elevator.getCurrentFloor()) {
        case 0:
            analogWrite(LED_1, CURRENT_FLOOR_BRIGHTNESS);
            break;
        case 1:
            analogWrite(LED_2, CURRENT_FLOOR_BRIGHTNESS);
            break;
        case 2:
            analogWrite(LED_3, CURRENT_FLOOR_BRIGHTNESS);
            break;
        case 3:
            analogWrite(LED_4, CURRENT_FLOOR_BRIGHTNESS);
            break;
    }

    // delay to prevent early reset of led
    delay(20);
}