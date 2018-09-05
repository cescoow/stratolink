#include <recoonI2C.h>

uint8_t gpsAddress = 0;
uint8_t slaves = 0;
uint8_t buf[256];
int LED = 58;

void setup() {
  pinMode(LED, OUTPUT);
  //pinMode(4, OUTPUT);
  //digitalWrite(4, HIGH);
  Serial.begin(9600);
  Serial.println("Discover TWI slaves...");

  twi_init(400000); // or 100000 Hz (standard mode)

  twiDiscoverSlaves();
  slaves = getSlavesCount();
  Serial.print("Total "); Serial.print(slaves, DEC); Serial.println(" i2c slave devices found");
  if (slaves) {
    for (uint8_t i = 0; i < slaves; i++) {
      Serial.print("\t0x");
      gpsAddress = getSlaveAddress(i);
      if (gpsAddress < 0x10) Serial.print("0");
      Serial.print(gpsAddress, HEX);
      Serial.println(" found");
    }
  }
}


void loop() {
  if (!slaves) return; // No TWI slave devices found. Return
  delay(200);
  uint16_t bytes = twiReadBytes(gpsAddress, 0xFD, (uint8_t *) buf, 2);
  if (!bytes) return; // got some TWI error. Return

  uint16_t totalBytes = ((uint16_t) buf[0] << 8) | buf[1];
  if (!totalBytes) return; // GPS not ready to send data. Return

  Serial.print("GPS is ready to transfer "); Serial.print(totalBytes, DEC); Serial.println(" bytes");
  digitalWrite(LED, HIGH);
  while (totalBytes) {
    uint16_t bytes2Read;
    if (totalBytes > 128) bytes2Read = 128; else bytes2Read = totalBytes;
    bytes = twiReadBytes(gpsAddress, 0xFF, (uint8_t *) buf, bytes2Read);
    for (uint8_t i = 0; i < bytes; i++) Serial.print(char(buf[i]));
    totalBytes -= bytes2Read;
  }
  Serial.println();
  digitalWrite(LED, LOW);

}
