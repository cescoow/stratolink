/* Cdigo teste para StratoLink. Implementado: Telemetria e envio de imagem, recepo e setagem de modos;
  Implementar:
  - Funo save_mode_log
  - Gerador de TM por structure
  - Watch Dog
  - Redundncia velocidade vert (bar + gps?)
  - Check de falha do barmetro
  - Deixar cod bonito
  Para o Mega Cores e Arduino IDE (ATMEGA2560v em 8MHz), pinagem "AVR pinout"
  Lista de Modos:
	0 - Auto - "auto\n" - Abertura autnoma da vlvula e trmino de voo
	1 - Photo - "send_photo\n" - Captura e envio de foto
	2 - Low Stab - "low_stab\n" - Estabilizao manual low
	3 - Med Stab - "medium_stab\n" - Estabilizao manual med
	4 - Full Stab - "full_stab\n" - Estabilizao manual full
	5 - Custom Stab - "custom_stab\n" - ND
	6 - Open Valve - "open_valve\n" - Abre vlvula manualmente
	7 - Close Valve - "close_valve\n" - Fecha vlvula manualmente
	8 - End Flight - "end_flight\n" - Termina voo
	9 - Bounce - "bounce\n" - Flutuao normal
	10 - Load String - "load_string\n" - Abre e fecha servo pra carregar corda
	11 - Reset - "reset\n" - Loop p/ reiniciar por WTD
	12 - Delay 15 min - "delay_15\n" - Posterga 15 min
	13 - Delay 30 min - "delay_30\n" - Posterga 30 min
	14 - Delay 60 min - "delay_60\n" - Posterga 60 min
	15 - Valve Test - "test\n" - Abre e fecha valvula p/ teste
*/

#include <arduino.h>
#include <SD.h>
#include <SPI.h>
#include <TinyGPS++.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <math.h>

/*TIAGO: inclui o inttypes*/
#include <inttypes.h>


#define PIC_PKT_LEN    128
#define PIC_FMT_VGA    7
#define PIC_FMT_CIF    5
#define PIC_FMT_OCIF   3
#define CAM_ADDR       0
#define PIC_FMT        PIC_FMT_VGA
#define SERVOMIN  150
#define SERVOMAX  600

/*TIAGO: Struct do pacote*/
typedef struct package {
  int32_t mode;
  uint8_t tk1; //ponto e virgula
  int32_t gps_isValid;
  uint8_t tk2;
  int32_t gps_lat;
  uint8_t tk3;
  int32_t gps_lng;
  uint8_t tk4;
  int32_t gps_hdop_value;
  uint8_t tk5;
  int32_t gps_satellites_value;
  uint8_t tk6;
  int32_t gps_altitude_meters;
  uint8_t tk7;
  int32_t gps_course_deg;
  uint8_t tk8;
  int32_t gps_speed_kmph;
  uint8_t tk9;
  int32_t bpm_temperature;
  uint8_t tk10;
  int32_t bpm_altitude;
  uint8_t tk11;
  int32_t bpm_pressure;
  uint8_t tk12;
  int32_t status_sd;
  uint8_t tk13;
  int32_t bar_speed;
  uint8_t tk14;
  int32_t is_open;
  uint8_t tk15;
  uint8_t end_char;
} package;


static const uint32_t GPSBaud = 9600;

File myFile;
TinyGPSPlus gps;
Adafruit_BMP085 bmp;
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

/*TIAGO: mudei os tipos de dodos para o inttypes
  Mas deixei comendado os antigos tipos
  byte byte_array[200];
  const byte cameraAddr = (CAM_ADDR << 5);
  const int buttonPin = A5;
  const long interval = 20000;
  unsigned long picTotalLen = 0;
  unsigned long previousMillis = 0;
  int picNameNum = 0;
  int counter = 0;
  int i = 0;
  int j = 0;
  int contador = 0;
  int buzzer = 21;
  int blue = 58;
  int red = 57;
  int mode = 0;
  int valve = 59;
  int servo = 0;
  int flight_duration = 5*60*60;
  float low_stab = 3; // m/s
  float med_stab = 1; // m/s
  float bar_speed = 100;
  float first_try_start = 15000;
  float first_try_stop = 18000;
  float second_try_start = 20000;
  float second_try_stop = 22000;
  float end_flight_alt = 22500;
  bool status_sd = false;
  bool manual_overide = false;
  bool updated_bar_speed = false;
  bool is_open = false;
  bool got_data = false;
  String string_data = "";
  String com_data = "";
  String picName = "pic0.jpg" ;
*/

/*TIAGO: inttypes*/
uint8_t byte_array[200];
const uint8_t cameraAddr = (CAM_ADDR << 5);
const int32_t buttonPin = A5;
const int32_t interval = 20000;
uint32_t picTotalLen = 0;
uint32_t previousMillis = 0;
int32_t picNameNum = 0;
int32_t counter = 0;
int32_t i = 0;
int32_t j = 0;
int32_t contador = 0;
int32_t buzzer = 21;
int32_t blue = 58;
int32_t red = 57;
int32_t mode = 0;
int32_t valve = 59;
int32_t servo = 0;
int32_t flight_duration = 5 * 60 * 60;
int32_t low_stab = 3; // m/s
int32_t med_stab = 1; // m/s
int32_t bar_speed = 100;
int32_t first_try_start = 15000;
int32_t first_try_stop = 18000;
int32_t second_try_start = 20000;
int32_t second_try_stop = 22000;
int32_t end_flight_alt = 22500;
uint8_t status_sd = false;
uint8_t manual_overide = false;
uint8_t updated_bar_speed = false;
uint8_t is_open = false;
uint8_t got_data = false;
String string_data = "";
String com_data = "";
String picName = "pic0.jpg" ;
const double launch_lat = -22.00492339;
const double launch_lon = -47.9347894;
const double launch_alt = 851;

/*TIAGO: inicializacao do pacote*/
package pkg;

/*********************************************************************/
void setup()
{
  Serial.begin(9600); // Cmera + interface PC
  Serial1.begin(9600); // LoRa
  Serial3.begin(9600); // GPS
  pinMode(11, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(valve, OUTPUT);
  digitalWrite(valve, 0);
  lora_config();
  set_cam_speed(); // Conf vel. serial da cmera
  if (!bmp.begin()) {
    Serial.println("Sensor nao encontrado!");
  }
  if (!SD.begin(11)) {
    Serial.print("Falha SD");
    return;
  }
  setGPS_DynamicModel6(); // Conf. GPS para airbone (at 50Km)
  //initialize(); // Inicia a cmera - Discutir se vamos deixar isso. Trava tudo se a cmera no responder no iniciar
  pwm.begin(); // Inicializa o driver pwm
  pwm.setPWMFreq(60); // Seta a freq. do driver
  delay(10);
  close_servo();
  start_song(); // Bonitin
}
/*********************************************************************/
void loop() // Rotina de modos devem ser implementadas para execuo unica, sem delays, em intervalos de smartDelay
{
  smartDelay(6000);
  if (got_data) {
    set_run_mode(com_data);
    Serial.println("Set mode");
    got_data = false;
  }
  run(mode);
  send_tm(); // Modo padro

}
/*********************************************************************/
void run(int mode) { // Implementar switch case de modos de voo. Os modos de voo devem ser executados dentro de perodos conhecidos
  switch (mode) {
    case 0:
      digitalWrite(blue, 1); // Implementar indicativos luminosos e sonoros pra cada modo
      digitalWrite(red, 0);
      mode_0();
      break;
    case 1:
      digitalWrite(blue, 1);
      digitalWrite(red, 1);
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
      digitalWrite(blue, 0);
      digitalWrite(red, 1);
      mode_8();
      break;
    case 9:
      mode_9();
      break;
    case 10:
      mode_10();
      break;
    case 11:
      mode_11();
      break;
    case 12:
      mode_12();
      break;
    case 13:
      mode_13();
      break;
    case 14:
      mode_14();
      break;
    default:
      mode_0();
      break;
  }

}
/*********************************************************************/
void open_valve() {
  pwm.setPWM(1, 4096, 0);
}
/*********************************************************************/
void close_valve() {
  pwm.setPWM(1, 0, 4096);
}
/*********************************************************************/
void end_flight() { // implementar checks e redundancias
  for (uint16_t pulselen = SERVOMAX; pulselen > SERVOMIN; pulselen--) { //abre o servo
    pwm.setPWM(servo, 0, pulselen);
  }
}
/*********************************************************************/
void mode_0() { // Auto - auto mode e retorna pra enviar TM - Adicionar indicativo LED + buzzer (green + high pitch)]
  if ((get_bar_alt() >= first_try_start) && (get_bar_alt() <= first_try_stop) && (updated_bar_speed) && (bar_speed >= low_stab)) {
    open_valve();
  } else {
    close_valve();
  }
  if ((get_bar_alt() >= second_try_start) && (get_bar_alt() <= second_try_stop) && (updated_bar_speed) && (bar_speed >= med_stab)) {
    open_valve();
  } else {
    close_valve();
  }
  if (get_bar_alt() >= end_flight_alt) {
    delay(1000); // esperar um pouco pra conferir
    if (get_bar_alt() >= end_flight_alt) {
      end_flight();
    }
  }
  if ((millis() / 1000) > flight_duration) {
    end_flight();
  }
}
/*********************************************************************/
void mode_1() { // Envia foto - Pronto
  send_photo();
  save_mode_log(1, 0);
  mode = 0;
}
/*********************************************************************/
void mode_2() { // Low stab - opera valvula com target de "" - definir var low_stab (3 m/s?)
  if ((updated_bar_speed) && (bar_speed >= low_stab)) {
    open_valve();
    save_mode_log(2, 1);
  } else {
    close_valve();
    save_mode_log(2, 0);
  }
}
/*********************************************************************/
void mode_3() { // Med stab - opera valvula com target de "" - definir var med_stab (1 m/s?)
  if ((updated_bar_speed) && (bar_speed >= med_stab)) {
    open_valve();
    save_mode_log(3, 1);
  } else {
    close_valve();
    save_mode_log(3, 0);
  }
}
/*********************************************************************/
void mode_4() { // Full stab - opera valvula com target de 0 m/s  - experimental - rodar antes de end_flight
  if ((updated_bar_speed) && (bar_speed >= 0.0)) {
    open_valve();
    save_mode_log(4, 1);
  } else {
    close_valve();
    save_mode_log(4, 0);
  }
}
/*********************************************************************/
void mode_5() { // custom_stab - discutir aplicabilidade
  save_mode_log(5, 0);
  mode = 0;
}
/*********************************************************************/
void mode_6() { // open_valve - hard mode - manual
  open_valve();
  save_mode_log(6, 1);
}
/*********************************************************************/
void mode_7() { // close_valve - hard mode - manual
  close_valve();
  save_mode_log(7, 0);
}
/*********************************************************************/
void mode_8() { // end_flight! Termina o voo. Implementar cut off. Implementar modo especial de descida?
  end_flight();
  save_mode_log(8, 0);
  mode = 0;
}
/*********************************************************************/
void mode_9() { // Bounce - sem acionamento da vlvula
  save_mode_log(9, 0);
}
/*********************************************************************/
void mode_10() { // Load String
  open_servo();
  delay(2000);
  close_servo();
  save_mode_log(10, 0);
  mode = 0;
}
/*********************************************************************/
void mode_11() { // Reset
  save_mode_log(11, 0);
  while (1) {
  }
}
/*********************************************************************/
void mode_12() { // Delay 15 min
  flight_duration = flight_duration + 15 * 60;
  save_mode_log(12, 0);
  mode = 0;
}
/*********************************************************************/
void mode_13() { // Delay 30 min
  flight_duration = flight_duration + 30 * 60;
  save_mode_log(13, 0);
  mode = 0;
}
/*********************************************************************/
void mode_14() { // Delay 60 min
  flight_duration = flight_duration + 60 * 60;
  save_mode_log(14, 0);
  mode = 0;
}
/*********************************************************************/
void mode_15() { // Abre e fecha valvula
  open_valve();
  delay(500);
  close_valve();
  save_mode_log(15, 0);
  mode = 0;
}


/*********************************************************************/


void open_servo(){ // Abre servo
  for (uint16_t pulselen = SERVOMAX; pulselen > SERVOMIN; pulselen--) { //abre o servo
    pwm.setPWM(servo, 0, pulselen);
  }
}
/*********************************************************************/
void close_servo() { // Fecha o servo - chamada no incio
  for (uint16_t pulselen = SERVOMIN; pulselen < SERVOMAX; pulselen++) {
    pwm.setPWM(servo, 0, pulselen);
  }
}
/*********************************************************************/
void save_mode_log(int function, int valve_status) { // Salva o modo de execuo em um .txt com o tempo de cod rodado
  if (valve_status == 0) {
    is_open = false;
  } else {
    is_open = true;
  }
  Serial.println(F("Save Mode"));
  myFile = SD.open("mode.txt", FILE_WRITE);
  if (myFile) {
    status_sd = true;
    myFile.print(function); myFile.print(";"); myFile.print(valve_status); myFile.print(";"); myFile.println(millis() / 1000);
    myFile.close();
    Serial.println(F("Gravado Mode"));
    tone(buzzer, 4000);
    delay(100);
    noTone(buzzer);
  } else {
    //digitalWrite(red, HIGH);
    tone(buzzer, 500);
    delay(1000);
    noTone(buzzer);
  }
}
/*********************************************************************/
static void smartDelay(unsigned long ms) { //Funo para ler string do GPS e LoRa e parar o cd por certo tempo
  float start_alt = get_bar_alt();
  unsigned long start = millis();
  do
  {
    while (Serial3.available() > 0) {
      gps.encode(Serial3.read());
    }
    receive_lora();
  }
  while ((millis() - start) < ms);
  float end_alt = get_bar_alt();
  bar_speed = (end_alt - start_alt) / (ms / 1000);
  updated_bar_speed = true;
}
/*********************************************************************/

float get_bar_alt() { // Media de 3 leituras p/ precisao
  float ref_alt = ((bmp.readAltitude() + bmp.readAltitude() + bmp.readAltitude()) / 3);
  return ref_alt;
}

/*********************************************************************/
void set_run_mode(String run_mode) { // Atualiza var global de modo de voo
  lora_send("Received comand:" + run_mode);
  Serial.println("Received comand:" + run_mode);
  if (run_mode.equals("auto\n")) {
    mode = 0;
    Serial.println("Mode set to: 0");
    lora_send("Mode set to: 0");
  }
  if (run_mode.equals("send_photo\n")) {
    mode = 1;
    lora_send("Mode set to: 1");
  }
  if (run_mode.equals("low_stab\n")) {
    mode = 2;
    lora_send("Mode set to: 2");
  }
  if (run_mode.equals("medium_stab\n")) {
    mode = 3;
    lora_send("Mode set to: 3");
  }
  if (run_mode.equals("full_stab\n")) {
    mode = 4;
    lora_send("Mode set to: 4");
  }
  if (run_mode.equals("custom_stab\n")) {
    mode = 5;
    lora_send("Target speed?");
    get_target_speed();
  }
  if (run_mode.equals("open_valve\n")) {
    mode = 6;
    lora_send("Mode set to: 6");
  }
  if (run_mode.equals("close_valve\n")) {
    mode = 7;
    lora_send("Mode set to: 7");
  }
  if (run_mode.equals("end_flight\n")) {
    mode = 8;
    lora_send("Mode set to: 8! Oh my! Going down!\n");
  }
  if (run_mode.equals("bounce\n")) {
    mode = 9;
    lora_send("Mode set to: 9\n");
  }
  if (run_mode.equals("load_string\n")) {
    mode = 10;
    lora_send("Mode set to: 10\n");
  }
  if (run_mode.equals("reset\n")) {
    mode = 11;
    lora_send("Mode set to: 11\n");
  }
  if (run_mode.equals("delay_15\n")) {
    mode = 12;
    lora_send("Mode set to: 12\n");
  }
  if (run_mode.equals("delay_30\n")) {
    mode = 13;
    lora_send("Mode set to: 13\n");
  }
  if (run_mode.equals("delay_60\n")) {
    mode = 14;
    lora_send("Mode set to: 14\n");
  }
  if (run_mode.equals("test\n")) {
    mode = 15;
    lora_send("Mode set to: 15\n");
  }
}

/*********************************************************************/
void get_target_speed() { // Construir funo que retorne a velocidade alvo em uma varivel global (receber do lora)
}
/*********************************************************************/

void lora_send(String data) { // Envia string via LoRa
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();
}
/*********************************************************************/
void receive_lora() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    com_data = "";
    got_data = true; {
      Serial.print("Pacote recebido");
      while (LoRa.available()) {
        com_data.concat((char)LoRa.read());
      }
    }
  }
}
/*********************************************************************/

void send_tm() //Funo para parser da NMEA, calculo dos parmetros do barmetro, montar e enviar a string de telemetria. Rever
{

  //TIAGO: Passando as leituras para variaveis
  int32_t mode = 3;
  int32_t gps_is_valid = gps.location.isValid();
  int32_t gps_lat = gps.location.lat();
  int32_t gps_lng = gps.location.lng();
  int32_t gps_hdop_value = gps.hdop.value();
  int32_t gps_satellites_value = gps.satellites.value();
  int32_t gps_altitude_meters = gps.altitude.meters();
  int32_t gps_course_deg = gps.course.deg();
  int32_t gps_speed_kmph = gps.speed.kmph();
  int32_t bpm_temperature = bmp.readTemperature();
  int32_t bpm_altitude = get_bar_alt();//func do Cesco
  int32_t bpm_pressure = bmp.readPressure();
  double distanceKm = gps.distanceBetween(gps.location.lat(),gps.location.lng(),launch_lat,launch_lon) / 1000;
  double course = gps.courseTo(launch_lat,launch_lon,gps_lat,gps_lng);
  int32_t elev = atan((gps_altitude_meters - launch_alt) / distanceKm * 1000);
  uint8_t end_char = 'x';

  //String Padro
  //(CIR;Valido;lat;lon;HDOP;Sats;alt_gps;direo;vel_kph;temp_bmp;alt_bmp)
  i = 0;
  string_data = "";
  string_data.concat("2;");
  /*TIAGO: Coloquei a variavel aqui*/
  if (gps_is_valid) {
    string_data.concat(1); string_data.concat(";");
  }
  else {
    string_data.concat(0); string_data.concat(";");
    Serial.print(F("INVALID"));
  }
  //2;1;-220008.53;-479019.78;0;0;0.00;0.00;0.33;0.00;30367.23;234;1;x
  string_data.concat(gps_lat * 10000); string_data.concat(";");
  string_data.concat(gps_lng * 10000); string_data.concat(";");
  string_data.concat(gps_hdop_value); string_data.concat(";");
  string_data.concat(gps_satellites_value); string_data.concat(";");
  string_data.concat(gps_altitude_meters); string_data.concat(";");
  string_data.concat(gps_course_deg); string_data.concat(";");
  string_data.concat(gps_speed_kmph); string_data.concat(";");
  string_data.concat(distanceKm); string_data.concat(";");
  string_data.concat(course); string_data.concat(";");
  string_data.concat(elev); string_data.concat(";");
  string_data.concat(bpm_temperature); string_data.concat(";");
  string_data.concat(bpm_altitude); string_data.concat(";");
  string_data.concat(bpm_pressure); string_data.concat(";");
  string_data.concat(status_sd); string_data.concat(";");
  string_data.concat(bar_speed); string_data.concat(";");
  string_data.concat(is_open); ; string_data.concat(";");
  string_data.concat(millis() / 1000);
  string_data.concat(";x");
  save_data();
  Serial.println(string_data);
  lora_send(string_data); // Envia a string de TM para o LoRa

  /*TIAGO: Montagem da struct do pacote*/
  pkg.mode = 3;
  pkg.tk1 = ';';
  pkg.gps_isValid = gps_is_valid;
  pkg.tk3 = ';';
  pkg.gps_lat = gps_lat;
  pkg.tk4 = ';';
  pkg.gps_lng = gps_lng;
  pkg.tk5 = ';';
  pkg.gps_hdop_value = gps_hdop_value;
  pkg.tk6 = ';';
  pkg.gps_satellites_value = gps_satellites_value;
  pkg.tk7 = ';';
  pkg.gps_altitude_meters = gps_altitude_meters;
  pkg.tk8 = ';';
  pkg.gps_course_deg = gps_course_deg;
  pkg.tk9 = ';';
  pkg.gps_speed_kmph = gps_speed_kmph;
  pkg.tk10 = ';';
  pkg.bpm_temperature = bpm_temperature;
  pkg.tk11 = ';';
  pkg.bpm_altitude = bpm_altitude;
  pkg.tk12 = ';';
  pkg.bpm_pressure = bpm_pressure;
  pkg.tk13 = ';';
  pkg.status_sd = status_sd;
  pkg.tk14 = ';';
  pkg.is_open = is_open;
  pkg.tk15 = ';';
  pkg.end_char = 'x';

  tone(buzzer, 2000);
  digitalWrite(blue, 1);
  delay(100);
  noTone(buzzer);
  digitalWrite(blue, 0);
  contador++;
  if (contador >= 4) {
    //send_photo();
    contador = 0;
  }
  //delay(800);
}

/*********************************************************************/

void send_photo() { // Funo que chama as rotinas de captura, transferncia, gravao no SD, leitura do arquivo, quebra de pacote e envio da imagem
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
void lora_config() { // Configura os parmetros do LoRa
  Serial.println("Iniciando LoRa");
  LoRa.setPins(10, 9, 2); //LoRa.setPins(ss, reset, dio0)
  if (!LoRa.begin(433.125E6)) {
    Serial.println("Falha LoRa");
    //while (1); Discutir - resetar wtd
  }
  LoRa.setSignalBandwidth(125E3);
  LoRa.setSpreadingFactor(11);
  LoRa.enableCrc();
}

/*********************************************************************/
void save_data() { // Grava a string padro global no SD
  //digitalWrite(red, LOW);
  //wdt_reset();

  Serial.println(F("Save."));
  myFile = SD.open("log.txt", FILE_WRITE);
  if (myFile) {
    status_sd = true;
    myFile.println(string_data);
    myFile.close();
    Serial.println(F("Gravado."));
    tone(buzzer, 3000);
    delay(100);
    noTone(buzzer);
  } else {
    //digitalWrite(red, HIGH);
    tone(buzzer, 800);
    delay(1000);
    noTone(buzzer);
  }
}

/*********************************************************************/
void read_data_sd() { // L o arquivo de imagem no SD, quebra em pacotes e envia
  myFile = SD.open(picName);
  int file_size = myFile.size();
  String start_string = "1;"; // Inicializador do modo de recepo de imagem na base
  start_string.concat(file_size);
  start_string.concat(";");
  start_string.concat(file_size / 200);
  start_string.concat(";x"); // Finalizador padro de string para o processing
  Serial1.print(start_string);
  delay(5000);
  String data_lora = "";
  if (myFile) {

    while (myFile.available()) {
      for (int j = 0; j <= 199; j++) {
        byte_array[j] = 0;
      }
      while (counter <= 199) {
        byte_array[counter] = myFile.read();
        counter++;
      }
      delay(10);
      LoRa.beginPacket();
      for (int i = 0; i <= 199; i++) {
        LoRa.write(byte_array[i]);
        //delay(2);
      }
      LoRa.endPacket();
      tone(buzzer, 1500);
      digitalWrite(red, 1);
      digitalWrite(blue, 1);
      delay(200);
      noTone(buzzer);
      digitalWrite(red, 0);
      digitalWrite(blue, 0);
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
void sendCmd(byte cmd[], int cmd_len) // Envia comandos pra cmera
{
  for (char i = 0; i < cmd_len; i++) Serial.write(cmd[i]);
}
/*********************************************************************/
int readBytes(char *dest, int len, unsigned int timeout) // L respostas da cmera
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
void initialize() // Inicializa a cmera e envia instrues de configurao
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
void preCapture() // Fuo pra setar as conf pr-captura da cmera
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

  while (SD.exists(picName)) {
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

void set_cam_speed() { // Configura serial da cmera
  byte comand[] = {0x55, 0x49, 0x01, 0x33, 0x23};
  int i = 0;
  while (i < 5) {
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

void capture_test() { // Funo teste - no t sendo chamada
  clearRxBuf();
  byte comand[] = { 0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN >> 8) & 0xff , 0};
  int i = 0;
  while (i < 7) {
    Serial.write(comand[i]);
    i++;
  }
  delay(1000);
}

/*********************************************************************/

void setGPS_DynamicModel6() // Configura GPS para modo airbone (at 50Km)
{
  int gps_set_sucess = 0;
  uint8_t setdm6[] = {
    0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
    0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC
  };
  while (!gps_set_sucess)
  {
    sendUBX(setdm6, sizeof(setdm6) / sizeof(uint8_t));
    gps_set_sucess = getUBX_ACK(setdm6);
  }
}
void sendUBX(uint8_t *MSG, uint8_t len) {
  Serial3.flush();
  Serial3.write(0xFF);
  _delay_ms(500);
  for (int i = 0; i < len; i++) {
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
  for (uint8_t ubxi = 2; ubxi < 8; ubxi++) {
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
void start_song() { // Musica legal
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

/*********************************************************************/

void setServoPulse(int n, double pulse) {
  double pulselength;
  pulselength = 1000000;
  pulselength /= 60;   // 60 Hz
  Serial.print(pulselength); Serial.println(" us por periodo");
  pulselength /= 4096;  // resoluo 12 bits
  Serial.print(pulselength); Serial.println(" us por bit");
  pulse *= 1000000;
  pulse /= pulselength;
  Serial.println(pulse);
  pwm.setPWM(n, 0, pulse);
}
