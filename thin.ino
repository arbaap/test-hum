#define TINY_GSM_MODEM_SIM800    

#include <Wire.h>
#include <TinyGsmClient.h>
#include <TinyGPS++.h>

#define echoPin 4                
#define trigPin 2                
#define vibrationMotorInput 12   
#define buzzerInput 14           
#define SIM800L_RX 27          
#define SIM800L_TX 26           
#define SIM800L_PWRKEY 4         
#define SIM800L_RST 5            
#define SIM800L_POWER 23         
#define GPS_RX 32            
#define GPS_TX 33               

#define THINGSPEAK_HOST "api.thingspeak.com"
#define THINGSPEAK_API_KEY "JKWIPCBNUJRN3E"

const char apn[] = "internet";  // APN
const char gprsUser[] = "";     // Username GPRS jika ada
const char gprsPass[] = "";     // Password GPRS jika ada

long duration, distance;
HardwareSerial SerialAT(1);
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
TinyGPSPlus gps;
HardwareSerial SerialGPS(2); 

void setup() {
  Serial.begin(115200);
  SerialAT.begin(115200, SERIAL_8N1, SIM800L_RX, SIM800L_TX);
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(vibrationMotorInput, OUTPUT);
  pinMode(buzzerInput, OUTPUT);
  pinMode(SIM800L_PWRKEY, OUTPUT);
  pinMode(SIM800L_RST, OUTPUT);
  pinMode(SIM800L_POWER, OUTPUT);

  digitalWrite(SIM800L_PWRKEY, LOW);
  digitalWrite(SIM800L_RST, HIGH);
  digitalWrite(SIM800L_POWER, HIGH);

  Serial.println("Initializing modem...");
  modem.restart();

  Serial.print("Connecting to APN: ");
  Serial.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println(" fail");
    while (true);
  }
  Serial.println(" OK");
}

void loop() {
  while (SerialGPS.available()) {
    gps.encode(SerialGPS.read());
  }

  if (gps.location.isUpdated()) {
    float latitude = gps.location.lat();
    float longitude = gps.location.lng();

    Serial.print("Latitude: ");
    Serial.println(latitude, 6);
    Serial.print("Longitude: ");
    Serial.println(longitude, 6);

    sendToThingspeak(latitude, longitude);
  }

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration / 58.2;
  String disp = String(distance);

  Serial.print("Distance: ");
  Serial.print(disp);
  Serial.println(" cm");

  if (distance < 150) {
    digitalWrite(vibrationMotorInput, HIGH);
    digitalWrite(buzzerInput, HIGH);
  } else {
    digitalWrite(vibrationMotorInput, LOW);
    digitalWrite(buzzerInput, LOW);
  }

  delay(1000);
}

void sendToThingspeak(float latitude, float longitude) {
  if (!client.connect(THINGSPEAK_HOST, 80)) {
    Serial.println("Connection to ThingSpeak failed");
    return;
  }

  String postStr = "api_key=";
  postStr += THINGSPEAK_API_KEY;
  postStr += "&field1=";
  postStr += String(latitude, 6);
  postStr += "&field2=";
  postStr += String(longitude, 6);
  postStr += "\r\n";

  client.print("POST /update HTTP/1.1\n");
  client.print("Host: api.thingspeak.com\n");
  client.print("Connection: close\n");
  client.print("Content-Type: application/x-www-form-urlencoded\n");
  client.print("Content-Length: ");
  client.print(postStr.length());
  client.print("\n\n");
  client.print(postStr);

  Serial.println("Data sent to ThingSpeak");

  delay(60000);
  client.stop();
}