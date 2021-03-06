#include <arduino.h>
#include <SD.h>
#include <SPI.h>

#define PIC_PKT_LEN    128
#define PIC_FMT_VGA    7
#define PIC_FMT_CIF    5
#define PIC_FMT_OCIF   3
#define CAM_ADDR       0
#define PIC_FMT        PIC_FMT_VGA

File myFile;

byte byte_array[200];
const byte cameraAddr = (CAM_ADDR << 5);  // addr
const int buttonPin = A5;                 // the number of the pushbutton pin
unsigned long picTotalLen = 0;            // picture length
int picNameNum = 0;
int counter = 0;
int M0 = 41;
int M1 = 40;
int aux = 39;

/*********************************************************************/
void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(11, OUTPUT);         // CS pin of SD Card Shiel
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(aux, INPUT);
  lora_config();
  digitalWrite(M0, 0);
  digitalWrite(M1, 0);
  set_cam_speed();
  if (!SD.begin(11)) {
    Serial.print("sd init failed");
    return;
  }
  initialize();
}
/*********************************************************************/
void loop()
{
  send_photo();
  delay(5000);
}
/*********************************************************************/

void send_photo(){
  int n = 0;
  delay(200);
  preCapture();
  Capture();
  //capture_test();
  GetData();
  delay(1000);
  read_data_sd();
}

/*********************************************************************/
void lora_config(){
  digitalWrite(M0, 1);
  digitalWrite(M1, 1);
  delay(160);
  byte comand[] = {0xC0, 0x00, 0x00, 0x3D, 0x0F, 0x44};
  int i = 0;
  while (i < 6){
    Serial1.write(comand[i]);
    i++;
  }
  delay(500);
  Serial1.flush();
  delay(2);
  Serial1.end();
  delay(500);

  Serial1.begin(115200);

}
/*********************************************************************/
void read_data_sd(){
  myFile = SD.open("PIC00.jpg");
  int file_size = myFile.size();
  String start_string = "img;";
  start_string.concat(file_size);
  start_string.concat(";");
  start_string.concat(file_size/200);
  start_string.concat(";x");
  Serial1.print(start_string);
  delay(5000);
  String data_lora = "";
  if (myFile) {
    //Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      for (int j=0; j <= 199; j++){
        byte_array[j] = 0;
      }
      while (counter <= 199){
        byte_array[counter] = myFile.read();
        counter++;
      }
      delay(10);
      for (int i=0; i <= 199; i++){
        Serial1.write(byte_array[i]);
        //delay(2);
      }
      delay(500);
      counter = 0;
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
/*********************************************************************/
void clearRxBuf()
{
  while (Serial.available())
  {
    Serial.read();
  }
}
/*********************************************************************/
void sendCmd(byte cmd[], int cmd_len)
{
  for (char i = 0; i < cmd_len; i++) Serial.write(cmd[i]);
}
/*********************************************************************/
int readBytes(char *dest, int len, unsigned int timeout)
{
  int read_len = 0;
  unsigned long t = millis();
  while (read_len < len)
  {
    while (Serial.available() < 1)
    {
      if ((millis() - t) > timeout)
      {
        return read_len;
      }
    }
    *(dest + read_len) = Serial.read();
    //Serial.write(*(dest + read_len));
    read_len++;
  }
  return read_len;
}
/*********************************************************************/
void initialize()
{
  byte cmd[] = {0xaa, 0x0d | cameraAddr, 0x00, 0x00, 0x00, 0x00} ;
  unsigned char resp[6];

  //Serial.print("initializing camera...");

  while (1)
  {
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 1000) != 6)
    {
      //Serial.print(".");
      continue;
    }
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x0d && resp[4] == 0 && resp[5] == 0)
    {
      if (readBytes((char *)resp, 6, 500) != 6) continue;
      if (resp[0] == 0xaa && resp[1] == (0x0d | cameraAddr) && resp[2] == 0 && resp[3] == 0 && resp[4] == 0 && resp[5] == 0) break;
    }
  }
  cmd[1] = 0x0e | cameraAddr;
  cmd[2] = 0x0d;
  sendCmd(cmd, 6);
  //Serial.println("\nCamera initialization done.");
}
/*********************************************************************/
void preCapture()
{
  byte cmd[] = { 0xaa, 0x01 | cameraAddr, 0x00, 0x07, 0x00, PIC_FMT };
  unsigned char resp[6];

  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x01 && resp[4] == 0 && resp[5] == 0) break;
  }
}
void Capture()
{
  byte cmd[] = { 0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN >> 8) & 0xff , 0};
  unsigned char resp[6];

  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x06 && resp[4] == 0 && resp[5] == 0) break;
  }
  cmd[1] = 0x05 | cameraAddr;
  cmd[2] = 0;
  cmd[3] = 0;
  cmd[4] = 0;
  cmd[5] = 0;
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x05 && resp[4] == 0 && resp[5] == 0) break;
  }
  cmd[1] = 0x04 | cameraAddr;
  cmd[2] = 0x1;
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x04 && resp[4] == 0 && resp[5] == 0)
    {
      if (readBytes((char *)resp, 6, 1000) != 6)
      {
        continue;
      }
      if (resp[0] == 0xaa && resp[1] == (0x0a | cameraAddr) && resp[2] == 0x01)
      {
        picTotalLen = (resp[3]) | (resp[4] << 8) | (resp[5] << 16);
        //Serial.print("picTotalLen:");
        //Serial.println(picTotalLen);
        break;
      }
    }
  }

}
/*********************************************************************/
void GetData()
{
  unsigned int pktCnt = (picTotalLen) / (PIC_PKT_LEN - 6);
  if ((picTotalLen % (PIC_PKT_LEN - 6)) != 0) pktCnt += 1;

  byte cmd[] = {0xaa, 0x0e | cameraAddr, 0x00, 0x00, 0x00, 0x00};
  unsigned char pkt[PIC_PKT_LEN];

  char picName[] = "pic00.jpg";
  //picName[3] = picNameNum / 10 + '0';
  //picName[4] = picNameNum % 10 + '0';

  if (SD.exists(picName))
  {
    SD.remove(picName);
  }

  myFile = SD.open(picName, FILE_WRITE);
  if (!myFile) {
    //Serial.println("myFile open fail...");
  }
  else {
    for (unsigned int i = 0; i < pktCnt; i++)
    {
      cmd[4] = i & 0xff;
      cmd[5] = (i >> 8) & 0xff;

      int retry_cnt = 0;
retry:
      delay(10);
      clearRxBuf();
      sendCmd(cmd, 6);
      uint16_t cnt = readBytes((char *)pkt, PIC_PKT_LEN, 200);

      unsigned char sum = 0;
      for (int y = 0; y < cnt - 2; y++)
      {
        sum += pkt[y];
      }
      if (sum != pkt[cnt - 2])
      {
        if (++retry_cnt < 100) goto retry;
        else break;
      }

      myFile.write((const uint8_t *)&pkt[4], cnt - 6);
      //if (cnt != PIC_PKT_LEN) break;
    }

    cmd[4] = 0xf0;
    cmd[5] = 0xf0;
    sendCmd(cmd, 6);
  }
  myFile.close();
  //picNameNum ++;
}

/*********************************************************************/

void set_cam_speed(){
  byte comand[] = {0x55, 0x49, 0x01, 0x33, 0x23};
    int i = 0;
    while (i < 5){
      Serial.write(comand[i]);
      i++;
    }
    delay(500);
    Serial.flush();
    delay(2);
    Serial.end();
    delay(500);

    Serial.begin(57600);
}

/*********************************************************************/

void capture_test(){
  clearRxBuf();
  byte comand[] = { 0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN >> 8) & 0xff , 0};
    int i = 0;
    while (i < 7){
      Serial.write(comand[i]);
      i++;
    }
    delay(1000);
}
