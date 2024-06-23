#define TINY_GSM_MODEM_SIM800

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGsmClient.h>



#define SOIL_MOISTURE_PIN 33
#define BME_SDA 18
#define BME_SCL 19
#define SDA_PIN 21
#define SCL_PIN 22
#define DMSpin 6
#define indikator 13
#define adcPin A0


#define MODEM_RST 5
#define MODEM_PWRKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26
#define MODEM_BAUD 115200

Adafruit_BME280 bme;
LiquidCrystal_I2C lcd(0x27, 16, 2);
TinyGsm modem(Serial1);
TinyGsmClient client(modem);


const char apn[] = "indosatgprs";  // APN for your SIM card
const char user[] = "";            // Usually empty
const char pass[] = "";            // Usually empty



unsigned long previousMillis = 0;
const long interval = 5000;
boolean firstDisplay = true;  // variabel untuk menandai tahapan tampilan pada LCD
int lastSoilMoisture = -1;    // Initialize with a value that won't match any valid reading
float lastReading = -1.0;     // Initialize lastReading for pH value

void setup() {
  Serial.begin(115200);
  Wire.begin(BME_SDA, BME_SCL);
  lcd.init();
  lcd.backlight();

  if (!bme.begin(0x76)) {
    Serial.println("Could not find BME280 sensor, check wiring!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor BME280");
    lcd.setCursor(0, 1);
    lcd.print("not found!");
    while (1)
      ;
  }


  setupGSM();
}

void setupGSM() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(10);

  // Initialize LCD
  Wire.begin(BME_SDA, BME_SCL);
  lcd.init();
  lcd.backlight();

  // Initialize BME280 sensor
  if (!bme.begin(0x76)) {
    Serial.println("Could not find BME280 sensor, check wiring!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor BME280");
    lcd.setCursor(0, 1);
    lcd.print("not found!");
    while (1)
      ;
  }

  // Initialize GSM module
  Serial1.begin(MODEM_BAUD, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  // Power on the modem
  pinMode(MODEM_PWRKEY, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWRKEY, LOW);
  digitalWrite(MODEM_POWER_ON, HIGH);
  delay(1000);
  digitalWrite(MODEM_PWRKEY, HIGH);

  // Reset modem
  pinMode(MODEM_RST, OUTPUT);
  digitalWrite(MODEM_RST, HIGH);
  delay(100);
  digitalWrite(MODEM_RST, LOW);
  delay(200);
  digitalWrite(MODEM_RST, HIGH);
  delay(10000);

  // Initialize modem
  Serial.println("Initializing modem...");
  modem.restart();
  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);

  // Connect to GSM network
  Serial.print("Connecting to network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" fail");
    while (true)
      ;
  }
  Serial.println(" success");

  // Connect to GPRS
  Serial.print("Connecting to ");
  Serial.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println(" fail");
    while (true)
      ;
  }
  Serial.println(" success");


  // Check signal quality
  int signalQuality = modem.getSignalQuality();
  Serial.print("Signal quality: ");
  Serial.print(signalQuality);
  Serial.println(" dBm");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);

    if (soilMoistureValue > 0 && soilMoistureValue != lastSoilMoisture) {
      lastSoilMoisture = soilMoistureValue;

      int soilMoisturePercentage = map(soilMoistureValue, 0, 4095, 100, 0);
      float temperature = bme.readTemperature();
      float humidity = bme.readHumidity();
      float pressure = bme.readPressure() / 100.0F;
      float pH = (-0.0139 * analogRead(adcPin)) + 7.0851;
      lastReading = pH;  // Update lastReading with the latest pH value

      Serial.println("=================================");
      Serial.println("Data saat sensor di tanah:");
      Serial.print("Nilai pH         : ");
      Serial.println(pH, 1);
      Serial.print("Kelembaban Tanah : ");
      Serial.println(soilMoisturePercentage);
      Serial.print("Temperature      : ");
      Serial.println(temperature);
      Serial.print("Humidity         : ");
      Serial.println(humidity);
      Serial.println("=================================");

      // Tampilkan data di LCD
      lcd.clear();
      if (firstDisplay) {
        lcd.setCursor(0, 0);
        lcd.print("Temp : ");
        lcd.print(temperature);
        lcd.print((char)223);
        lcd.print("C");

        lcd.setCursor(0, 1);
        lcd.print("Hum  : ");
        lcd.print(humidity);
        lcd.print("%");
        firstDisplay = false;
      } else {
        lcd.setCursor(0, 0);
        lcd.print("Soil : ");
        lcd.print(soilMoisturePercentage);
        lcd.print("%");

        lcd.setCursor(0, 1);
        lcd.print("pH   : ");
        lcd.print(pH, 1);
        firstDisplay = true;
      }

      sendDataAll(String(soilMoisturePercentage), String(temperature), String(humidity), String(pH));



      Serial.print("\nVia GSM ");

      // Check signal quality periodically
      int signalQuality = modem.getSignalQuality();
      Serial.print("\nSignal quality: ");
      Serial.print(signalQuality);
      Serial.println(" dBm");
    }
  }
}

void sendDataAll(String kelembapanTanah, String temperature, String humidity, String pHTanah) {
  String server = "test-hum.vercel.app";
  String url = "/api/data/kirimData";
  int port = 80;

  if (!client.connect(server.c_str(), port)) {
    Serial.println("Connection to server failed!");
    return;
  }

  String payload = "{\"kelembapan_tanah\": \"" + kelembapanTanah + "\",\"temperature\": \"" + temperature + "\", \"humidity\": \"" + humidity + "\", \"pH_tanah\": \"" + pHTanah + "\"}";

  Serial.println("Sending data via GSM:");
  Serial.println(payload);

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " + payload.length() + "\r\n" +
               "\r\n" +
               payload);

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read HTTP response
  String response = client.readString();
  Serial.println("Response from server:");
  Serial.println(response);

  // Check for HTTP redirection (308 Permanent Redirect)
  if (response.indexOf("HTTP/1.1 308") != -1 || response.indexOf("HTTP/1.0 308") != -1) {
    Serial.println("Redirecting...");
    // Find location header
    int locationStart = response.indexOf("Location: ");
    if (locationStart != -1) {
      int locationEnd = response.indexOf("\r\n", locationStart);
      if (locationEnd != -1) {
        String newUrl = response.substring(locationStart + 10, locationEnd);
        newUrl.trim();
        Serial.print("New URL: ");
        Serial.println(newUrl);

        // Close current connection
        client.stop();

        // Make a new connection to the new URL
        int newPort = 443; // example port for HTTPS
        if (!client.connect(newUrl.c_str(), newPort)) {
          Serial.println("Connection to new URL failed!");
          return;
        }

        // Update URL and send payload again
        url = newUrl + url; // append the original endpoint
        client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                     "Host: " + server + "\r\n" +
                     "Content-Type: application/json\r\n" +
                     "Content-Length: " + payload.length() + "\r\n" +
                     "\r\n" +
                     payload);

        // Wait for response from new URL
        timeout = millis();
        while (client.available() == 0) {
          if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
          }
        }

        // Read response from new URL
        response = client.readString();
        Serial.println("Response from new URL:");
        Serial.println(response);
      }
    }
  } else {
    // Check HTTP response code
    if (response.indexOf("HTTP/1.1 200 OK") == -1) {
      Serial.println("Failed to send data via GSM.");
    } else {
      Serial.println("Data sent successfully via GSM.");
    }
  }

  client.stop();
}
