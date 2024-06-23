#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>

#define SOIL_MOISTURE_PIN 33
#define BME_SDA 18
#define BME_SCL 19
#define SDA_PIN 21
#define SCL_PIN 22
#define DMSpin  6
#define indikator 13
#define adcPin A0

Adafruit_BME280 bme;

const char* ssid = "Giri29"; // Ganti dengan nama WiFi Anda
const char* password = "aksara2910"; // Ganti dengan kata sandi WiFi Anda
const char* thingSpeakApiKey = "7OKF6S7F65BJ0XOT"; // Ganti dengan kunci API ThingSpeak Anda
const char* server = "api.thingspeak.com";
const String url = "/update?api_key=";

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Alamat I2C dan ukuran LCD

unsigned long previousMillis = 0;
const long interval = 5000;
boolean firstDisplay = true; // variabel untuk menandai tahapan tampilan pada LCD
int lastSoilMoisture = -1; // Initialize with a value that won't match any valid reading
float lastReading = -1.0; // Initialize lastReading for pH value

void setup() {
  Serial.begin(9600);
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
    while (1);
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("Connecting  to");
    lcd.setCursor(3, 1);
    lcd.print("WiFi....");
    Serial.println("Connecting to WiFi...");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" WiFi  Connected");
  lcd.setCursor(5, 1);
  lcd.print("....");
  Serial.println("Connected to WiFi");
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
      float pH = (-0.0139 * analogRead(adcPin)) + 7.7851;
      lastReading = pH; // Update lastReading with the latest pH value

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
    }
  }
}

void sendDataAll(String kelembapanTanah, String temperature, String humidity, String pHTanah) {
  HTTPClient http;
  http.begin("https://server-phtanah.vercel.app/api/data/kirimData");
  http.addHeader("Content-Type", "application/json");

  String payload = "{\"kelembapan_tanah\": \"" + kelembapanTanah + "\",\"temperature\": \"" + temperature + "\", \"humidity\": \"" + humidity + "\", \"pH_tanah\": \"" + pHTanah + "\"}";

  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    Serial.print("Data Terkirim ke Server! Kode respons: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Gagal mengirim data. Kode respons: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}