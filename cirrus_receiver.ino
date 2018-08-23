#include <SPI.h>
#include <LoRa.h>
String string_data = "";
int i = 0;

void setup()
{
  Serial.begin(9600);
  //  ss.begin(9600);
  //Serial3.begin(GPSBaud);

  Serial.println("LoRa Sender");
  LoRa.setPins(7, 9, 2); //LoRa.setPins(ss, reset, dio0)
  if (!LoRa.begin(433.125E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(8);
  LoRa.setSpreadingFactor(7);
  LoRa.enableCrc();
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while (LoRa.available()) {
      //string_data.concat(LoRa.read());
      /*unsigned short data = ((LoRa.read()));
      unsigned char char1;
      unsigned char char2;
      char1 = data & 0xFF;
      char2 = data >> 8;
      data = (char2 << 8) | char1;
      Serial.write(data);
      */
      //byte a = LoRa.read();
      Serial.write(LoRa.read());
    }
    //string_data.concat("\n");
    //Serial.print(i);
    i++;
    //Serial.print(string_data);
    //Serial.print(LoRa.packetRssi()); 
    //Serial.print(";x");
    string_data = "";
  }
}
