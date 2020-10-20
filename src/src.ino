#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define API_KEY "tPmAT5Ab3j7F9"
#define DEVICE_ID 1
#define SENSOR_TEMP_ID 1
#define SENSOR_HUMID_ID 2
#define SENSOR_PRES_ID 3

// API Key to verify
String apiKeyValue = "tPmAT5Ab3j7F9";

String deviceId = "1";

String sensorId1 = "1"; // temperature
String sensorId2 = "2"; // humidity
String sensorId3 = "3"; // pressure

// This is the Lets Encrypt certficate 
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n" \
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
"DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n" \
"SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n" \
"GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \
"AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n" \
"q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n" \
"SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n" \
"Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n" \
"a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n" \
"/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n" \
"AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n" \
"CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n" \
"bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n" \
"c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n" \
"VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n" \
"ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n" \
"MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n" \
"Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n" \
"AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n" \
"uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n" \
"wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n" \
"X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n" \
"PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n" \
"KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n" \
"-----END CERTIFICATE-----\n";

// Replace with your network credentials
const char* ssid     = "Sde-Guest";
const char* password = "";

// Domain for API
const String serverName = "https://eux-teknik-smartgreenhouse.rasmusbundsgaard.dk";

/*#include <SPI.h>
#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5*/

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;  // I2C
//Adafruit_BME280 bme(BME_CS);  // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);  // software SPI

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

//  bool status = bme.begin(0x76);
//  if (!status) {
//    Serial.println("Could not find a valid BME280 sensor, check wiring or change I2C address!");
//    while (1);
//  }
}

void loop() {
  // Check WiFi connection status
  if(WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure *client = new WiFiClientSecure;

    if (!client) {
      Serial.println("Unable to create client");
      delay(30000);
      return;
    }

    client->setCACert(rootCACertificate);

    {
      HTTPClient http;

      http.useHTTP10(true);

      // Domain name with full URL to 'API'
      if (http.begin(serverName + "/esp.php?api_key=" + API_KEY)) {  
        float sensorTempValue = 23.4;
        float sensorHumidValue = 50.0;
        float sensorPresValue = 1.0;
        String timeStamp = "1603186961";

        // Data format: did:x,sid:y,v:z,t:a
        String postData = \
          "did:" + String(DEVICE_ID) + "," + "sid:" + String(SENSOR_TEMP_ID)  + "," + "v:" + sensorTempValue  + "," + "t:" + timeStamp + "\n" \
          "did:" + String(DEVICE_ID) + "," + "sid:" + String(SENSOR_HUMID_ID) + "," + "v:" + sensorHumidValue + "," + "t:" + timeStamp + "\n" \
          "did:" + String(DEVICE_ID) + "," + "sid:" + String(SENSOR_PRES_ID)  + "," + "v:" + sensorPresValue  + "," + "t:" + timeStamp;

        http.addHeader("Content-Type", "text/plain");

        // Send HTTP POST request
        int httpResponseCode = http.POST(postData);

        if (httpResponseCode>0) {
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
        } else {
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
        }

        // Free resources
        http.end();
      } else {
        Serial.println("[HTTPS] Unable to connect");
      }
    }

    delete client;
  } else {
    Serial.println("WiFi Disconnected");
  }

  // Send an HTTP POST request every 30 seconds
  delay(10000);  
}
