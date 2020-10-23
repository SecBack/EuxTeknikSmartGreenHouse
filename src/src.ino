#include <WiFi.h>
#include "time.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

#define API_HOST "https://eux-teknik-smartgreenhouse.rasmusbundsgaard.dk"
#define API_KEY "lEWQZliMMZgASzEwsOKtauUkWToAuiShRrJ7SWN3BHtGbMbaKMYT3dGiSu0E" // This is also unique to each device
#define SENSOR_TEMP_ID 1
#define SENSOR_HUMID_ID 2
#define SENSOR_PRES_ID 3

#define WIFI_SSID "Sde-Guest"
#define WIFI_PSWD ""

#define SEALEVELPRESSURE_HPA (1013.25)

#define PUSH_INTERVAL 10000 // 10 secs

// Clock / NTP setup
#define NTP_SERVER "pool.ntp.org"
#define NTP_SERVER_ALT "time.nist.gov"

// This is the Lets Encrypt root certficate
#define ROOT_CA_CERTIFICATE \
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
"-----END CERTIFICATE-----\n"

Adafruit_BMP280 bmp; // I2C

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PSWD);
  Serial.println("Connecting");

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Show IP to serial
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Init and get the time
  setClock();

  // Could also pass in a Wire library object like &Wire2
  if (!bmp.begin(BMP280_ADDRESS_ALT)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring or change I2C address!");
    while (1);
  }

  // Set sampling mode for BMP280
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop() {
  // Check WiFi connection status
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Not connected to WiFi");
    delay(10000); // 10 secs

    return;
  }

  bool successPush = pushData();
  if (!successPush) {
    delay(10000); // 10 secs
    return;
  }

  // If API request was successfull we interval every x minutes
  delay(PUSH_INTERVAL);
}

bool pushData() {
  WiFiClientSecure *client = new WiFiClientSecure;
  if (!client) {
    Serial.println("Unable to create TLS client");
    return false;
  }

  // Set the CA certifcate to check the server's certifcate is signed by the trusted CA.
  client->setCACert(ROOT_CA_CERTIFICATE);

  bool success = false; // Successful flag

  {
    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is

    HTTPClient https;

    // Domain name with full URL to 'API'
    if (!https.begin(*client, String(API_HOST) + "/api/v1/sensorData?api_token=" + API_KEY)) {
      Serial.println("[HTTPS] Unable to connect");
      success = false; // Not successful
    } else {
      // Get sensor values
      float sensorTempValue = bmp.readTemperature();
      float sensorHumidValue = random(0, 101) / 1.0F; // Apparently BMP280 doesnt have humidity sensor
      float sensorPresValue = bmp.readPressure();
      String timeStamp = getCurrentTimestamp();

      // Data format: sid:x,v:y,t:z (sensor_id:x,value:y,timestamp:z)
      String postData = \
        "sid:" + String(SENSOR_TEMP_ID)  + "," + "v:" + sensorTempValue  + "," + "t:" + timeStamp + "\n" \
        "sid:" + String(SENSOR_HUMID_ID) + "," + "v:" + sensorHumidValue + "," + "t:" + timeStamp + "\n" \
        "sid:" + String(SENSOR_PRES_ID)  + "," + "v:" + sensorPresValue  + "," + "t:" + timeStamp;

      // Set correct content type that the API expects
      https.addHeader("Content-Type", "text/plain");

      // Send HTTP POST request
      int responseCode = https.POST(postData);

      Serial.print("Response code: ");
      Serial.println(responseCode);

      // Not successful if error code is not between 200-299
      if (responseCode < 200 || responseCode >= 300) {
        success = false;
      }
    }

    // Free resources
    https.end();
  }

  delete client;

  return success; // Return false if error or true on success
}

void setClock() {
  configTime(0, 0, NTP_SERVER, NTP_SERVER_ALT);

  Serial.print("Waiting for NTP time sync: ");
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

String getCurrentTimestamp() {
  // https://www.esp32.com/viewtopic.php?f=2&t=7328
  struct timeval tv;
  gettimeofday(&tv, NULL);
  uint64_t milliseconds = tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;

  const int NUM_DIGITS = log10(milliseconds) + 1;
  char str[NUM_DIGITS + 1];

  char* retStr = uintToStr(milliseconds, str);

  return String(retStr);
}

char* uintToStr(const uint64_t num, char *str) {
  // https://forum.arduino.cc/index.php?topic=378359.0
  uint8_t i = 0;
  uint64_t n = num;

  do {
    i++;
  } while( n /= 10 );

  str[i] = '\0';
  n = num;

  do {
    str[--i] = ( n % 10 ) + '0';
  } while (n /= 10);

  return str;
}
