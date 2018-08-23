#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <SD.h>
#include <TinyGPS++.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <SoftwareSerial.h>
static const int RXPin = 10, TXPin = 11;

static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
//Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

//SoftwareSerial ss(RXPin, TXPin);

int i = 0;
int j = 0;
bool status_sd = false;
String string_data = "";

Adafruit_BMP085 bmp;
File myFile;

void setup()
{
  pinMode(58, OUTPUT);
  Serial.begin(9600);
  Serial3.begin(9600);
//  ss.begin(9600);
  //Serial3.begin(GPSBaud);
  if (!bmp.begin()){
    Serial.println("Sensor nao encontrado !!");
  }
  
  Serial.println("LoRa Sender");
  LoRa.setPins(10, 9, 2); //LoRa.setPins(ss, reset, dio0) 
  if (!LoRa.begin(433.125E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSignalBandwidth(125E3);
  LoRa.setSpreadingFactor(11);
  LoRa.enableCrc();
  if (!SD.begin(4)) {
      Serial.println("Falha no SD.");
      //digitalWrite(red, HIGH);
      status_sd = false;
      delay(1000);
    }else{ 
      Serial.println("SD inicializado.");
  }
}

void loop()
{
  //Serial.println("loop");
  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial3.available() > 0)
    if (gps.encode(Serial3.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}

void displayInfo()
{
  //String Padrão
  //(CIR;Valido;lat;lon;HDOP;Sats;alt_gps;direção;vel_kph;temp_bmp;alt_bmp)
  i = 0;
  string_data = "";
  string_data.concat("CIR;");
  if (gps.location.isValid()){
    string_data.concat(1); string_data.concat(";"); 
  }
  else{
    string_data.concat(0); string_data.concat(";");
    Serial.print(F("INVALID"));
  }
  digitalWrite(58, HIGH);
  
  string_data.concat(gps.location.lat()*10000); string_data.concat(";");
  string_data.concat(gps.location.lng()*10000); string_data.concat(";");
  string_data.concat(gps.hdop.value());string_data.concat(";");
  string_data.concat(gps.satellites.value());string_data.concat(";");
  string_data.concat(gps.altitude.meters());string_data.concat(";");
  string_data.concat(gps.course.deg());string_data.concat(";");
  string_data.concat(gps.speed.kmph());string_data.concat(";");
  string_data.concat(bmp.readTemperature());string_data.concat(";");
  string_data.concat(bmp.readAltitude());string_data.concat(";");
  string_data.concat(bmp.readPressure());string_data.concat(";");
  string_data.concat(status_sd);string_data.concat(";");
  Serial.println(string_data);
  LoRa.beginPacket();
  LoRa.print(string_data);
  LoRa.endPacket();
  save_data();
  digitalWrite(58, LOW);
  
  while (j < 800){
    j++;
    tone(21, j*3);
    delay(1);
  }
  noTone(21);
  j = 0;
}

void save_data(){
  //digitalWrite(red, LOW);
  //wdt_reset();
   
  Serial.println(F("Save."));
  myFile = SD.open("log.txt", FILE_WRITE);
  if(myFile){
    status_sd = true;
    myFile.println(string_data);
    myFile.close();
    Serial.println(F("Gravado."));
  }else{
    //digitalWrite(red, HIGH); 
  }
}
