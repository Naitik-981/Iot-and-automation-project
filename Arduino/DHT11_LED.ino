#include <WiFiS3.h>
#include "secrets.h"
#include "ThingSpeak.h"
#include "dht_module.h"
#include <Arduino_JSON.h>  // Include the Arduino JSON library for LED control

// Pin configuration and timing intervals
#define DHT11_PIN1 2               // Pin for the first DHT11 sensor
#define DHT11_PIN2 4               // Pin for the second DHT11 sensor
#define READ_INTERVAL 15000        // Interval (in ms) between sensor readings
#define THINGSPEAK_INTERVAL 15000  // Interval (in ms) between ThingSpeak updates

// LED pin configuration
const int ledPin = 13;  // GPIO pin for the LED

// Wi-Fi and ThingSpeak setup
WiFiClient client;
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
unsigned long myChannelNumber = SECRET_CH_ID_1;
unsigned long ledChannelNumber = SECRET_CH_ID_2;
const char* myWriteAPIKey = SECRET_WRITE_APIKEY;
const char* myReadAPIKey = SECRET_READ_APIKEY;
const char* channelID = "2717430";  // ThingSpeak channel ID for LED control
int status = WL_IDLE_STATUS;

// DHT sensor objects
DHTSensor dht1(DHT11_PIN1, DHT11);
DHTSensor dht2(DHT11_PIN2, DHT11);

// Time tracking variables
unsigned long lastReadTime = 0;

// Function to print WiFi network information
void printNetworkInfo() {
  Serial.print("WiFi Status: ");
  Serial.println(WiFi.status());
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Function to connect to WiFi
void connectToWiFi() {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Warning: Firmware is outdated.");
  }

  while (status != WL_CONNECTED) {
    Serial.print("Connecting to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }

  Serial.println("Connected to WiFi");
  printNetworkInfo();
}

// Function to reconnect WiFi if disconnected
void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    connectToWiFi();
  }
}

// Function to write data to ThingSpeak
void updateThingSpeak(float humidity1, float temperature1, float humidity2, float temperature2) {
  ThingSpeak.setField(1, humidity1);
  ThingSpeak.setField(2, temperature1);
  ThingSpeak.setField(3, humidity2);
  ThingSpeak.setField(4, temperature2);

  int response = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (response == 0) {
    Serial.println("Sensor Data Uploaded.");
  } else {
    Serial.println("Problem updating channel. HTTP error code: " + String(response));
  }
}

// Function to read and process data from a single sensor
void processSensorData(DHTSensor& sensor, float& humidity, float& temperature) {
  humidity = sensor.getHumidity();
  temperature = sensor.getTemperature();

  if (!sensor.isDataValid(humidity, temperature)) {
    Serial.println("Failed to read from sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("% | Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");
  }
}

// Function to read the `field5` value and control the LED
void controlLED() {
  // Construct the URL for the GET request
  String url = String("/channels/") + String(ledChannelNumber) + "/fields/1/last.json";
  // Connect to the ThingSpeak server
  if (client.connect("api.thingspeak.com", 80)) {
    Serial.println("Connected to ThingSpeak server for LED control!");
    // Send the HTTP GET request
    client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: api.thingspeak.com\r\n" + "Connection: close\r\n\r\n");

    // Read the response from the server
    String response = "";
    while (client.connected() || client.available()) {
      if (client.available()) {
        response += client.readString();
      }
    }
    client.stop();
    // Parse the JSON response
    int jsonStart = response.indexOf("{");
    if (jsonStart != -1) {
      String jsonString = response.substring(jsonStart);  // Extract JSON part of the response
      JSONVar data = JSON.parse(jsonString);              // Parse the JSON string

      if (JSON.typeof(data) == "undefined") {
        Serial.println("Failed to parse JSON for LED control!");
      } else {
        // Check if 'field5' exists in the JSON response
        if (data.hasOwnProperty("field1")) {
          String fieldValue = (const char*)data["field1"];
          fieldValue.trim();  // Remove any whitespace or extra characters

          Serial.println("LED Value: " + fieldValue);
          
          // Control the LED based on the `field5` value
          if (fieldValue.equals("1")) {
            digitalWrite(ledPin, HIGH);  // Turn on the LED
            Serial.println("LED ON");
          } else {
            digitalWrite(ledPin, LOW);  // Turn off the LED
            Serial.println("LED OFF");
          }
        } else {
          Serial.println("field1 not found in JSON!");
        }
      }
    } else {
      Serial.println("Invalid JSON response for LED control!");
    }

  } else {
    Serial.println("Connection to ThingSpeak failed for LED control!");
  }
}

void setup() {
  Serial.begin(9600);        // Initialize Serial communication
  connectToWiFi();           // Connect to WiFi
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  dht1.begin();              // Initialize first DHT sensor
  dht2.begin();              // Initialize second DHT sensor

  // Initialize the LED pin as output
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  // Start with the LED off
}

void loop() {
  reconnectWiFi();
  unsigned long currentTime = millis();

  if (currentTime - lastReadTime >= READ_INTERVAL) {
    float humidity1, temperature1, humidity2, temperature2;

    Serial.println("Reading from DHT11 sensors...");
    processSensorData(dht1, humidity1, temperature1);
    processSensorData(dht2, humidity2, temperature2);

    Serial.println("Updating ThingSpeak...");
    updateThingSpeak(humidity1, temperature1, humidity2, temperature2);
    // Control LED based on the latest ThingSpeak field value
    controlLED();
    lastReadTime = currentTime;
  }
}
