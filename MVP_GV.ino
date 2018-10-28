/* Código teste para StratoLink. Implementado: Telemetria e envio de imagem, recepção e setagem de modos;
Implementar:
  - Variáveis globais das funções
  - Funções de modos
  - Rotina de troca de modo automática
  - Função save_mode_log
  - Gerador de TM por structure
  - Função cut_off
  - Watch Dog
  - Case switch modos
  - Redundância velocidade vert (bar + gps?)
  - Deixar cod bonito
Para o Mega Cores e Arduino IDE (ATMEGA2560v em 8MHz), pinagem "AVR pinout"
*/

#include <arduino.h>
#include <SD.h>
#include <SPI.h>
#include <TinyGPS++.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>


#define PIC_PKT_LEN    128
#define PIC_FMT_VGA    7
#define PIC_FMT_CIF    5
#define PIC_FMT_OCIF   3
#define CAM_ADDR       0
#define PIC_FMT        PIC_FMT_VGA

static const uint32_t GPSBaud = 9600;

File myFile;
TinyGPSPlus gps;
Adafruit_BMP085 bmp;

byte byte_array[200];
const byte cameraAddr = (CAM_ADDR << 5);
const int buttonPin = A5;
const long interval = 20000;
unsigned long picTotalLen = 0;
unsigned long previousMillis = 0;
int picNameNum = 0;
int counter = 0;
int M0 = 41;
int M1 = 40;
int aux = 39;
int i = 0;
int j = 0;
int contador = 0;
int buzzer = 21;
int blue = 58;
int red = 57;
int mode = 0;
float low_stab = 3; // m/s
float med_stab = 1; // m/s
float bar_speed = 100;
bool status_sd = false;
bool manual_overide = false;
bool updated_bar_speed = false;
bool is_open = false;
bool got_data = false;
String string_data = "";
String com_data = "";
String picName = "pic0.jpg" ;

/*********************************************************************/
void setup()
{
  Serial.begin(9600); // Câmera + interface PC
  Serial1.begin(9600); // LoRa
  Serial3.begin(9600); // GPS
  pinMode(11, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(aux, INPUT);
  lora_config();
  digitalWrite(M0, 0); // Pino de modo LoRa
  digitalWrite(M1, 0); // Pino de modo LoRa
  set_cam_speed(); // Conf vel. serial da câmera
  if (!bmp.begin()){
    Serial.println("Sensor nao encontrado!");
  }
  if (!SD.begin(11)) {
    Serial.print("Falha SD");
    return;
  }
  setGPS_DynamicModel6(); // Conf. GPS para airbone (até 50Km)
  initialize(); // Inicia a câmera - Discutir se vamos deixar isso. Verificar se não trava a câmera se não iniciar
  start_song(); // Bonitin
}
/*********************************************************************/
void loop()
{
  smartDelay(2000);
  if (got_data){
    set_run_mode(com_data);
    got_data = false;
  }
  run(mode);
  send_tm();

}
/*********************************************************************/
void run(int mode){ // Implementar switch case de modos de voo. Os modos de voo devem ser executados dentro de períodos conhecidos
  switch (mode) {
    case 0:
      mode_0();
      break;
    case 1:
      mode_1();
      break;
    case 2:
      mode_2();
      break;
    case 3:
      mode_3();
      break;
    case 4:
      mode_4();
      break;
    case 5:
      mode_5();
      break;
    case 6:
      mode_6();
      break;
    case 7:
      mode_7();
      break;
    case 8:
      mode_8();
      break;
    case 9:
      mode_9();
      break;
    default:
      mode_0();
      break;
  }

}
/*********************************************************************/
void open_valve(){

}
/*********************************************************************/
void close_valve(){

}
/*********************************************************************/
void mode_0(){ // Auto - auto mode e retorna pra enviar TM - Adicionar indicativo LED + buzzer (green + high pitch)

}
/*********************************************************************/
void mode_1(){ // Envia foto - Pronto
  send_photo();
  save_mode_log(1, 0);
}
/*********************************************************************/
void mode_2(){ // Low stab - opera valvula com target de "" - definir var low_stab (3 m/s?)
  if ((updated_bar_speed) && (bar_speed >= low_stab)){
    open_valve();
    save_mode_log(2, 1);
  }else{
    close_valve();
    save_mode_log(2, 0);
  }
}
/*********************************************************************/
void mode_3(){ // Med stab - opera valvula com target de "" - definir var med_stab (1 m/s?)
  if ((updated_bar_speed) && (bar_speed >= med_stab)){
    open_valve();
    save_mode_log(3, 1);
  }else{
    close_valve();
    save_mode_log(3, 0);
  }
}
/*********************************************************************/
void mode_4(){ // Full stab - opera valvula com target de 0 m/s  - experimental - rodar antes de end_flight
  if ((updated_bar_speed) && (bar_speed >= 0.0)){
    open_valve();
    save_mode_log(4, 1);
  }else{
    close_valve();
    save_mode_log(4, 0);
  }
}
/*********************************************************************/
void mode_5(){ // custom_stab - discutir aplicabilidade
  save_mode_log(5, 0);
}
/*********************************************************************/
void mode_6(){ // open_valve - hard mode - manual
  open_valve();
  save_mode_log(6, 1);
}
/*********************************************************************/
void mode_7(){ // close_valve - hard mode - manual
  close_valve();
  save_mode_log(7, 0);
}
/*********************************************************************/
void mode_8(){ // end_flight! Termina o voo. Implementar cut off. Implementar modo especial de descida?
  save_mode_log(8, 0);
}
/*********************************************************************/
void mode_9(){ // Por enquanto nada
  save_mode_log(9, 0);
}
/*********************************************************************/
void save_mode_log(int function, int valve_status){ // Salva o modo de execução em um .txt com o tempo de cod rodado
  if (valve_status == 0){
    is_open = false;
  }else{
    is_open = true;
  }
}
/*********************************************************************/
static void smartDelay(unsigned long ms){ //Função para ler string do GPS e LoRa e parar o cód por certo tempo
  float start_alt = get_bar_alt();
  unsigned long start = millis();
  do
  {
    while (Serial3.available() > 0){
      gps.encode(Serial3.read());
    }
    if (digitalRead(aux) == 1 && Serial1.available() > 0){
      com_data = "";
      got_data = true;
      while(Serial1.available() > 0){
        com_data.concat(Serial1.read());
      }
    }
  }
  while ((millis() - start) < ms);
  float end_alt = get_bar_alt();
  bar_speed = (end_alt - start_alt)/(ms/1000);
  updated_bar_speed = true;
}
/*********************************************************************/

float get_bar_alt(){ // Media de 3 leituras p/ precisao
  float ref_alt = ((bmp.readAltitude() + bmp.readAltitude() + bmp.readAltitude())/3);
  return ref_alt;
}

/*********************************************************************/
void set_run_mode(String run_mode){ // Atualiza var global de modo de voo
  lora_send("Received comand:" + run_mode);
  if (run_mode.equals("auto")){
    mode = 0;
    lora_send("Mode set to: 0");
  }
  if (run_mode.equals("send_photo")){
    mode = 1;
    lora_send("Mode set to: 1");
  }
  if (run_mode.equals("low_stab")){
    mode = 2;
    lora_send("Mode set to: 2");
  }
  if (run_mode.equals("medium_stab")){
    mode = 3;
    lora_send("Mode set to: 3");
  }
  if (run_mode.equals("full_stab")){
    mode = 4;
    lora_send("Mode set to: 4");
  }
  if (run_mode.equals("custom_stab")){
    mode = 5;
    lora_send("Target speed?");
    get_target_speed();
  }
  if (run_mode.equals("open_valve")){
    mode = 6;
    lora_send("Mode set to: 6");
  }
  if (run_mode.equals("close_valve")){
    mode = 7;
    lora_send("Mode set to: 7");
  }
  if (run_mode.equals("end_flight")){
    mode = 8;
    lora_send("Mode set to: 8! Oh my! Going down!");
  }
}

/*********************************************************************/
void get_target_speed(){ // Construir função que retorne a velocidade alvo em uma variável global (receber do lora)
}
/*********************************************************************/

void lora_send(String data){ // Envia string via LoRa
  while (digitalRead(aux) == 0){ // Verifica se lora não está ocupado
  }
  if (digitalRead(aux) == 1){ // Checar mais uma vez por redundância
    Serial1.print(data);
  }
}
/*********************************************************************/

void send_tm() //Função para parser da NMEA, calculo dos parâmetros do barômetro, montar e enviar a string de telemetria. Rever
{
  //String Padrão
  //(CIR;Valido;lat;lon;HDOP;Sats;alt_gps;direção;vel_kph;temp_bmp;alt_bmp)
  i = 0;
  string_data = "";
  string_data.concat("2;");
  if (gps.location.isValid()){
    string_data.concat(1); string_data.concat(";");
  }
  else{
    string_data.concat(0); string_data.concat(";");
    Serial.print(F("INVALID"));
  }
 //2;1;-220008.53;-479019.78;0;0;0.00;0.00;0.33;0.00;30367.23;234;1;x
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
  string_data.concat(millis()/1000);
  string_data.concat(";x");
  save_data();
  Serial1.print(string_data); // Envia a string de TM para o LoRa
  tone(buzzer, 2000);
  digitalWrite(blue,1);
  delay(100);
  noTone(buzzer);
  digitalWrite(blue,0);
  contador++;
  if (contador >= 4){
    //send_photo();
    contador = 0;
  }
  //delay(800);
}

/*********************************************************************/

void send_photo(){ // Função que chama as rotinas de captura, transferência, gravação no SD, leitura do arquio, quebra de pacote e envio da imagem
  int n = 0;
  delay(200);
  preCapture();
  Capture();
  //capture_test();
  GetData();
  delay(2000);
  read_data_sd();
  delay(5000);
}

/*********************************************************************/
void lora_config(){ // Configura os parâmetros do LoRa
  digitalWrite(M0, 1);
  digitalWrite(M1, 1);
  delay(160);
  byte comand[] = {0xC0, 0x00, 0x00, 0x39, 0x10, 0x44};
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
void save_data(){ // Grava a string padrão global no SD
  //digitalWrite(red, LOW);
  //wdt_reset();

  Serial.println(F("Save."));
  myFile = SD.open("log.txt", FILE_WRITE);
  if(myFile){
    status_sd = true;
    myFile.println(string_data);
    myFile.close();
    Serial.println(F("Gravado."));
    tone(buzzer, 3000);
    delay(100);
    noTone(buzzer);
  }else{
    //digitalWrite(red, HIGH);
    tone(buzzer, 800);
    delay(1000);
    noTone(buzzer);
  }
}

/*********************************************************************/
void read_data_sd(){ // Lê o arquivo de imagem no SD, quebra em pacotes e envia
  myFile = SD.open(picName);
  int file_size = myFile.size();
  String start_string = "1;"; // Inicializador do modo de recepção de imagem na base
  start_string.concat(file_size);
  start_string.concat(";");
  start_string.concat(file_size/200);
  start_string.concat(";x"); // Finalizador padrão de string para o processing
  Serial1.print(start_string);
  delay(5000);
  String data_lora = "";
  if (myFile) {

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
      tone(buzzer, 1500);
      digitalWrite(red,1);
      digitalWrite(blue,1);
      delay(200);
      noTone(buzzer);
      digitalWrite(red,0);
      digitalWrite(blue,0);
      while(digitalRead(aux) == 0){ // Importante! "aux" fica em "low" enquanto o LoRa estiver enviando ou recebendo algo. Utilizar para rotina de envio
      }
      counter = 0;
    }
    myFile.close();
  } else {
    Serial.println("Falha no SD - Imagem");
  }
}
/*********************************************************************/
void clearRxBuf() // Limpa o buffer Serial
{
  while (Serial.available())
  {
    Serial.read();
  }
}
/*********************************************************************/
void sendCmd(byte cmd[], int cmd_len) // Envia comandos pra câmera
{
  for (char i = 0; i < cmd_len; i++) Serial.write(cmd[i]);
}
/*********************************************************************/
int readBytes(char *dest, int len, unsigned int timeout) // Lê respostas da câmera
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
void initialize() // Inicializa a câmera e envia instruções de configuração
{
  byte cmd[] = {0xaa, 0x0d | cameraAddr, 0x00, 0x00, 0x00, 0x00} ;
  unsigned char resp[6];

  while (1)
  {
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 1000) != 6)
    {
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
}
/*********************************************************************/
void preCapture() // Fução pra setar as conf pré-captura da câmera
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
/*********************************************************************/
void Capture() // Envia comando pra captura da imagem
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
void GetData() // Grava pacotes recebidos da camera
{
  unsigned int pktCnt = (picTotalLen) / (PIC_PKT_LEN - 6);
  if ((picTotalLen % (PIC_PKT_LEN - 6)) != 0) pktCnt += 1;

  byte cmd[] = {0xaa, 0x0e | cameraAddr, 0x00, 0x00, 0x00, 0x00};
  unsigned char pkt[PIC_PKT_LEN];

  while (SD.exists(picName)){
    picNameNum++;
    picName = "pic" ;
    picName.concat(picNameNum);
    picName.concat(".jpg");
    //SD.remove(picName);
    //picName[3] = picNameNum / 10 + '0';
    //picName[4] = picNameNum % 10 + '0';
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

void set_cam_speed(){ // Configura serial da câmera
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

void capture_test(){ // Função teste - não tá sendo chamada
  clearRxBuf();
  byte comand[] = { 0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN >> 8) & 0xff , 0};
    int i = 0;
    while (i < 7){
      Serial.write(comand[i]);
      i++;
    }
    delay(1000);
}

/*********************************************************************/

void setGPS_DynamicModel6() // Configura GPS para modo airbone (até 50Km)
{
 int gps_set_sucess=0;
 uint8_t setdm6[] = {
 0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06,
 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C,
 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC };
 while(!gps_set_sucess)
 {
 sendUBX(setdm6, sizeof(setdm6)/sizeof(uint8_t));
 gps_set_sucess=getUBX_ACK(setdm6);
 }
}
void sendUBX(uint8_t *MSG, uint8_t len) {
 Serial3.flush();
 Serial3.write(0xFF);
 _delay_ms(500);
 for(int i=0; i<len; i++) {
 Serial3.write(MSG[i]);
 }
}
boolean getUBX_ACK(uint8_t *MSG) {
 uint8_t b;
 uint8_t ackByteID = 0;
 uint8_t ackPacket[10];
 unsigned long startTime = millis();

// Construct the expected ACK packet
 ackPacket[0] = 0xB5; // header
 ackPacket[1] = 0x62; // header
 ackPacket[2] = 0x05; // class
 ackPacket[3] = 0x01; // id
 ackPacket[4] = 0x02; // length
 ackPacket[5] = 0x00;
 ackPacket[6] = MSG[2]; // ACK class
 ackPacket[7] = MSG[3]; // ACK id
 ackPacket[8] = 0; // CK_A
 ackPacket[9] = 0; // CK_B

// Calculate the checksums
 for (uint8_t ubxi=2; ubxi<8; ubxi++) {
 ackPacket[8] = ackPacket[8] + ackPacket[ubxi];
 ackPacket[9] = ackPacket[9] + ackPacket[8];
 }

while (1) {

// Test for success
 if (ackByteID > 9) {
 // All packets in order!
 return true;
 }

// Timeout if no valid response in 3 seconds
 if (millis() - startTime > 3000) {
 return false;
 }

// Make sure data is available to read
 if (Serial3.available()) {
 b = Serial3.read();

// Check that bytes arrive in sequence as per expected ACK packet
 if (b == ackPacket[ackByteID]) {
 ackByteID++;
 }
 else {
 ackByteID = 0; // Reset and look again, invalid order
 }
 }
 }
}

/*********************************************************************/
void start_song(){ // Musica legal
  tone(buzzer, 3000);
  delay(100);
  tone(buzzer, 800);
  delay(200);
  tone(buzzer, 100);
  delay(50);
  tone(buzzer, 800);
  delay(100);
  tone(buzzer, 100);
  delay(50);
  tone(buzzer, 2000);
  delay(100);
  tone(buzzer, 100);
  delay(50);
  tone(buzzer, 2000);
  delay(100);
  tone(buzzer, 100);
  delay(50);
  tone(buzzer, 3000);
  delay(200);
  tone(buzzer, 800);
  delay(100);
  tone(buzzer, 100);
  delay(50);
  tone(buzzer, 3000);
  delay(100);
  tone(buzzer, 100);
  delay(50);
  tone(buzzer, 2000);
  delay(300);
  tone(buzzer, 100);
  delay(50);
  noTone(buzzer);
}
