#include <Arduino.h>
#include <LittleFS.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;

#define FORMAT_LITTLEFS_IF_FAILED true

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

const char *server = "basubin-api.vercel.app";
const int port = 8000;
const char *endpoint = "/api/server";

// File path for storing credentials
const char *filePath = "/credential.txt";

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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

// Fungsi untuk menampilkan teks secara terpusat di layar OLED
void displayTextCentered(const char *text, uint8_t textSize, uint16_t textColor)
{
    // Menghapus buffer display
    display.clearDisplay();

    // Mengatur ukuran teks dan warna
    display.setTextSize(textSize);
    display.setTextColor(textColor);

    // Menghitung lebar dan tinggi teks untuk menengahkan teks
    int16_t x1, y1;
    uint16_t textWidth, textHeight;
    display.getTextBounds("Jam", 0, 0, &x1, &y1, &textWidth, &textHeight);

    // Menetapkan kursor ke posisi tengah
    int16_t x = (SCREEN_WIDTH - textWidth) / 2;
    int16_t y = (SCREEN_HEIGHT - textHeight) / 2;
    display.setCursor(x, y);

    // Menampilkan teks di layar
    display.print("Jam");

    // Menampilkan perubahan di layar
    display.display();
}
void displayLoading(const char *message)
{
    // Clear the display buffer
    display.clearDisplay();

    // Set text size and color
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Calculate the width of the text to center it
    int16_t x1, y1;
    uint16_t textWidth, textHeight;
    display.getTextBounds(message, 0, 0, &x1, &y1, &textWidth, &textHeight);

    // Set cursor to the center position
    display.setCursor((SCREEN_WIDTH - textWidth) / 2, (SCREEN_HEIGHT - textHeight) / 2);
    display.print(message);

    // Create a rotating loading animation with two circles
    static float frame = 0;
    int8_t radius = 5;
    int8_t circleRadius = 2;
    int8_t centerX = SCREEN_WIDTH / 2;
    int8_t centerY = (SCREEN_HEIGHT / 2) + textHeight + 10; // Adjust the gap by changing the value 10

    // Calculate the positions for the rotating circles
    int8_t x1_pos = centerX + radius * cos(frame);
    int8_t y1_pos = centerY + radius * sin(frame);
    int8_t x2_pos = centerX + radius * cos(frame + PI);
    int8_t y2_pos = centerY + radius * sin(frame + PI);

    // Draw the rotating circles
    display.fillCircle(x1_pos, y1_pos, circleRadius, SSD1306_WHITE);
    display.fillCircle(x2_pos, y2_pos, circleRadius, SSD1306_WHITE);
    frame += PI / 6; // Increase frame for smoother animation (12 fps)
    if (frame >= 2 * PI)
    {
        frame -= 2 * PI;
    }

    // Display the changes on the screen
    display.display();

    // Wait before the next frame (approximately 12 fps)
    delay(83);
}

void scanWifi()
{

    Serial.println("Wifi scanning...");
    displayTextCentered("Wifi scanning...", 1, SSD1306_WHITE);
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

void bootFunction()
{
    // Loop utama yang dijalankan ketika tidak terhubung ke WiFi
    display.clearDisplay();
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 300000)
    {

        // Tambahkan kode loop di sini
        // notify changed value
        if (!deviceConnected)
        {
            displayLoading("Waiting to Connect");
        }
        if (deviceConnected)
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                scanWifi();
            }
            delay(3000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
        }
        // disconnecting
        if (!deviceConnected && oldDeviceConnected)
        {
            Serial.println("Device disconnected.");
            displayTextCentered("Device Disconnected", 1, SSD1306_WHITE);
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
            Serial.println("Device connected");
            displayTextCentered("Device Connected", 1, SSD1306_WHITE);
        }
    }
}

void saveCredentialsToFS(const char *ssid, const char *password)
{
    // Buka file env.txt untuk menulis
    File file = LittleFS.open("/env.txt", "w");

    if (!file)
    {
        Serial.println("Failed to open file /env.txt for writing.");
        return;
    }

    // Tulis SSID dan password ke dalam file
    file.println(ssid);
    file.println(password);

    // Tutup file
    file.close();

    Serial.println("SSID and Password Saved");
}

void readCredentialsFromFS(char *ssid, char *password)
{
    // Buka file env.txt untuk membaca
    File file = LittleFS.open("/env.txt", "r");

    if (!file)
    {
        Serial.println("Failed to open file /env.txt for reading.");
        return;
    }

    // Baca SSID dari file
    if (file.available())
    {
        String ssidFromFile = file.readStringUntil('\n');
        ssidFromFile.trim(); // Menghapus spasi tambahan jika ada
        ssidFromFile.toCharArray(ssid, ssidFromFile.length() + 1);
    }

    // Baca password dari file
    if (file.available())
    {
        String passwordFromFile = file.readStringUntil('\n');
        passwordFromFile.trim(); // Menghapus spasi tambahan jika ada
        passwordFromFile.toCharArray(password, passwordFromFile.length() + 1);
    }

    // Tutup file
    file.close();

    Serial.println("SSID and Password Read from File:");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("Password: ");
    Serial.println(password);

    // Coba untuk terhubung menggunakan SSID dan password yang sudah terbaca
    WiFi.begin(ssid, password);
    unsigned long startAttemptTime = millis();
    while (millis() - startAttemptTime < 3000)
    {
        displayLoading("Connecting");
    }

    // Tambahkan penanganan status koneksi di sini jika diperlukan
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("failed to connect");
        displayTextCentered("Failed to Connect", 1, SSD1306_WHITE);
        delay(2000);
        LittleFS.remove("/env.txt");
        Serial.println("env.txt deleted");
    }
    else
    {
        Serial.println("success to connect");
        displayTextCentered("Connected", 1, SSD1306_WHITE);
        delay(2000);
    }
}

String parseResponseForId(String response)
{
    // Example: Parse JSON response to extract _id
    // Modify according to your API's response format
    // This is a simplified example assuming JSON format
    String idMarker = "\"_id\":";
    int idIndex = response.indexOf(idMarker);
    if (idIndex == -1)
    {
        Serial.println("No _id found in response");
        return "";
    }

    // Find end of _id value
    int idStart = idIndex + idMarker.length();
    int idEnd = response.indexOf(",", idStart);
    if (idEnd == -1)
    {
        idEnd = response.indexOf("}", idStart);
    }

    if (idEnd == -1)
    {
        Serial.println("Invalid _id format in response");
        return "";
    }

    // Extract _id
    String idValue = response.substring(idStart, idEnd);
    idValue.trim();

    return idValue;
}

String sendPostRequest()
{
    WiFiClient client;

    if (!client.connect(server, port))
    {
        Serial.println("Connection to API failed");
        return "";
    }

    // Construct POST data
    String postData = "some_data_to_send";

    // Send POST request
    client.print(String("POST ") + endpoint + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Content-Type: application/x-www-form-urlencoded\r\n" +
                 "Content-Length: " + postData.length() + "\r\n" +
                 "Connection: close\r\n\r\n" +
                 postData);

    unsigned long timeout = millis();
    while (client.available() == 0)
    {
        if (millis() - timeout > 5000)
        {
            Serial.println("Client Timeout !");
            client.stop();
            return "";
        }
    }

    // Read response from server
    String response = "";
    while (client.available())
    {
        response += client.readStringUntil('\r');
    }

    // Extract _id from response (example)
    String responseId = parseResponseForId(response);

    // Close connection
    client.stop();

    return responseId;
}

void saveIdToFile(String id)
{
    File file = LittleFS.open(filePath, "w");
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    // Write _id to file
    file.println(id);
    file.close();

    Serial.println("Saved _id to file");
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
            WiFi.begin(ssid, password);
            displayTextCentered("SDSOn", 1, SSD1306_WHITE);
            unsigned long startAttemptTime = millis();
            while (millis() - startAttemptTime < 3000)
            {
                displayLoading("Connecting");
            }

            // Optional: Add code to handle connection status, timeouts, etc.
            if (WiFi.status() != WL_CONNECTED)
            {
                Serial.println("failed to connect");
                displayTextCentered("Failed to Connect", 1, SSD1306_WHITE);
                pOutputCharacteristic->setValue("failed");
                pOutputCharacteristic->notify();
                delay(2000);
                bootFunction();
            }

            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.println("success to connect");
                displayTextCentered("Connected", 1, SSD1306_WHITE);
                pOutputCharacteristic->setValue("success");
                pOutputCharacteristic->notify();
                saveCredentialsToFS(ssid, password);

                // Make HTTP POST request to API
                String responseId = sendPostRequest();
                Serial.print("Response _id: ");
                Serial.println(responseId);
            }
        }
    }
};

void startingPoint()
{
    if (LittleFS.exists("/env.txt"))
    {
        char ssid[32];
        char password[32];

        readCredentialsFromFS(ssid, password);
    }

    if (!LittleFS.exists("/env.txt") || WiFi.status() != WL_CONNECTED)
    {
        displayTextCentered("Turn On Bluetoth", 1, SSD1306_WHITE);
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
        displayTextCentered("Bluetoth On", 1, SSD1306_WHITE);

        // boot function
        bootFunction();
    }
    else
    {
    }
}

void sendToAPI(float temperature, float pressure, float altitude, float humidity)
{
    WiFiClient client;

    if (!client.connect(server, port))
    {
        Serial.println("Connection to API failed");
        return;
    }

    String url = String(endpoint) + "?temperature=" + String(temperature, 2) + "&pressure=" + String(pressure, 2) + "&altitude=" + String(altitude, 2) + "&humidity=" + String(humidity, 2);

    Serial.print("Sending data to API: ");
    Serial.println(url);

    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");

    unsigned long timeout = millis();
    while (client.available() == 0)
    {
        if (millis() - timeout > 5000)
        {
            Serial.println("Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read response from server
    while (client.available())
    {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }

    Serial.println();
    Serial.println("Data sent to API");

    client.stop();
}

// Function to display sensor readings on OLED
void printValues()
{
    // Read sensor data
    float temperature = bme.readTemperature();
    float pressure = bme.readPressure() / 100.0F;
    float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    float humidity = bme.readHumidity();

    // Display on OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("Temperature: ");
    display.print(temperature);
    display.println(" C");

    display.setCursor(0, 10);
    display.print("Pressure: ");
    display.print(pressure);
    display.println(" hPa");

    display.setCursor(0, 20);
    display.print("Altitude: ");
    display.print(altitude);
    display.println(" m");

    display.setCursor(0, 30);
    display.print("Humidity: ");
    display.print(humidity);
    display.println(" %");

    display.display();

    // Print to Serial for debugging
    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Pressure = ");
    Serial.print(pressure);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(altitude);
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.println();

    // Send sensor data to API
    sendToAPI(temperature, pressure, altitude, humidity);
}

void setup()
{
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);

    // Inisialisasi LittleFS
    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED))
    {
        Serial.println("LittleFS Mount Failed");
        return;
    }
    else
    {
        Serial.println("Little FS Mounted Successfully");
    }

    // Check OLED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C addr 0x3C (for the 128x32)
    // init done
    display.display();
    delay(1000);
    display.setTextColor(WHITE);

    Wire.begin();

    startingPoint();

    unsigned status;
    status = bme.begin(0x76);
}

void loop()
{
    printValues();
    delay(30000);
}