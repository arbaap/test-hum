#define TINY_GSM_MODEM_SIM800

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGsmClient.h>
#include <HTTPClient.h>

#define SOIL_MOISTURE_PIN 33
#define BME_SDA 18
#define BME_SCL 19
#define SDA_PIN 21
#define SCL_PIN 22
#define DMSpin  6
#define indikator 13
#define adcPin A0
#define MODEM_RST 5
#define MODEM_PWRKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26
#define MODEM_BAUD 115200

const char apn[] = "internet";  
const char user[] = "";         
const char pass[] = "";         

Adafruit_BME280 bme;
LiquidCrystal_I2C lcd(0x27, 16, 2);
TinyGsm modem(Serial1);
TinyGsmClient client(modem);

unsigned long previousMillis = 0;
const long interval = 5000;
boolean firstDisplay = true;
int lastSoilMoisture = -1;
float lastReading = -1.0;

void setup() {
  Serial.begin(115200);
  delay(10);

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

  Serial1.begin(MODEM_BAUD, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  pinMode(MODEM_PWRKEY, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWRKEY, LOW);
  digitalWrite(MODEM_POWER_ON, HIGH);
  delay(1000);
  digitalWrite(MODEM_PWRKEY, HIGH);

  pinMode(MODEM_RST, OUTPUT);
  digitalWrite(MODEM_RST, HIGH);
  delay(100);
  digitalWrite(MODEM_RST, LOW);
  delay(200);
  digitalWrite(MODEM_RST, HIGH);
  delay(10000);

  Serial.println("Initializing modem...");
  modem.restart();
  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);

  Serial.print("Connecting to network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" fail");
    while (true);
  }
  Serial.println(" success");

  Serial.print("Connecting to ");
  Serial.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println(" fail");
    while (true);
  }
  Serial.println(" success");

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
      float pH = (-0.0139 * analogRead(adcPin)) + 7.7851;
      lastReading = pH;

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

      int signalQuality = modem.getSignalQuality();
      Serial.print("Signal quality: ");
      Serial.print(signalQuality);
      Serial.println(" dBm");
    }
  }
}

void sendDataAll(String kelembapanTanah, String temperature, String humidity, String pHTanah) {
  HTTPClient http;
  
  http.begin("https://server-phtanah.vercel.app/api/data/kirimData");
  http.addHeader("Content-Type", "application/json");

  String payload = "{\"kelembapan_tanah\": \"" + kelembapanTanah + "\",\"temperature\": \"" + temperature + "\", \"humidity\": \"" + humidity + "\", \"pH_tanah\": \"" + pHTanah + "\"}";

  http.followRedirects(true);

  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    Serial.print("Data Terkirim ke Server! Kode respons: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Gagal mengirim data. Kode respons: ");
    Serial.println(httpResponseCode);
  }

  // Menutup koneksi HTTP
  http.end();
}
