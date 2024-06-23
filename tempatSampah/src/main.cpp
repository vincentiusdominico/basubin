#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include <WiFi.h>

BLEServer *pServer = NULL;
BLECharacteristic *pOutputCharacteristic = NULL;
BLECharacteristic *pInputCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

const int ledPin = 2; // Use the appropriate GPIO pin for your setup

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "19b10000-e8f2-537e-4f6c-d104768a1214"
#define OUTPUT_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
#define INPUT_CHARACTERISTIC_UUID "19b10002-e8f2-537e-4f6c-d104768a1214"

void scanWifi()
{
    Serial.println("Wifi scanning...");
    int networksFound = WiFi.scanNetworks();
    delay(2000); // give the wifi scan the chance to get things ready

    JsonDocument doc;

    if (networksFound == 0)
    {
        Serial.println("No networks found");
    }
    else
    {
        Serial.print("Found ");
        Serial.print(networksFound);
        Serial.println(" networks");

        for (int i = 0; i < networksFound; ++i)
        {
            JsonObject wifiInfo = doc.add<JsonObject>();
            wifiInfo["ssid"] = WiFi.SSID(i);
            wifiInfo["rssi"] = WiFi.RSSI(i);
        }

        String jsonStr;
        serializeJson(doc, jsonStr);

        pOutputCharacteristic->setValue(jsonStr.c_str());
        pOutputCharacteristic->notify();
        Serial.println("WiFi scan results sent to client");
    }
}

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pInputCharacteristic)
    {
        std::string value = pInputCharacteristic->getValue();
        if (value.length() == 1)
        {
            Serial.print("Characteristic event, written: ");
            int intValue = value[0];
            Serial.println(intValue); // Print the integer value

            if (intValue == 1)
            {
                scanWifi();
                digitalWrite(ledPin, HIGH);
            }
            else
            {
                digitalWrite(ledPin, LOW);
            }
        }
        else
        {
            // Handle JSON string
            String jsonString = String(value.c_str());
            Serial.println("Received JSON: " + jsonString);

            JsonDocument doc; // Adjust size as needed
            DeserializationError error = deserializeJson(doc, jsonString);

            if (error)
            {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
                return;
            }

            const char *ssid = doc["ssid"];
            const char *password = doc["password"];

            Serial.print("SSID: ");
            Serial.println(ssid);
            Serial.print("Password: ");
            Serial.println(password);

            // Connect to WiFi with provided credentials
            // WiFi.begin(ssid, password);

            // Optional: Add code to handle connection status, timeouts, etc.
        }
    }
};

void setup()
{
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);

    // Create the BLE Device
    BLEDevice::init("ESP32");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pOutputCharacteristic = pService->createCharacteristic(
        OUTPUT_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE);

    // Create the ON button Characteristic
    pInputCharacteristic = pService->createCharacteristic(
        INPUT_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE);

    // Register the callback for the ON button characteristic
    pInputCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    pOutputCharacteristic->addDescriptor(new BLE2902());
    pInputCharacteristic->addDescriptor(new BLE2902());

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify...");
}

void loop()
{
    // notify changed value
    if (deviceConnected)
    {
        pOutputCharacteristic->setValue("HELLO");
        pOutputCharacteristic->notify();
        value++;
        Serial.print("New value notified: ");
        Serial.println(value);
        delay(3000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected)
    {
        Serial.println("Device disconnected.");
        delay(500);                  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected)
    {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
        Serial.println("Device Connected");
    }
}