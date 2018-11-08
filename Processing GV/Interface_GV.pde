// implementar https://www.tigoe.com/pcomp/code/Processing/23/ Cescooo
import processing.serial.*;
import controlP5.*;
import processing.core.PApplet;
import de.fhpotsdam.unfolding.UnfoldingMap;
import de.fhpotsdam.unfolding.geo.Location;
import de.fhpotsdam.unfolding.utils.MapUtils;
import de.fhpotsdam.unfolding.marker.SimplePointMarker;
import de.fhpotsdam.unfolding.providers.OpenStreetMap.*;
import de.fhpotsdam.unfolding.utils.ScreenPosition;
//import de.fhpotsdam.unfolding.providers.PrecipitationClassic.*;
import de.fhpotsdam.unfolding.providers.Microsoft;
import de.fhpotsdam.unfolding.providers.Yahoo;
//import de.fhpotsdam.unfolding.providers.OpenWeatherProvider;
import processing.opengl.*;
import saito.objloader.*;
import processing.serial.*;
import ddf.minim.*;

PImage img;
PImage bkg;
PImage balloon;

AudioSnippet beep;
AudioSnippet send_song;
Minim minim;

int linefeed = 10;
int carriageReturn = 13;
int sensorValue = 0;
int i = 0;
int dir = 0;
int temp_bmp = 0;
int alt_bmp = 0;
int time = 0;
int time_out = 0;
int file_size = 0;
int packet_number = 0;
int received_bytes = 0;
int new_counter = 0;
int indexer = 0;
int header = int(random(1000));
int scale = 1;
int running_time = 0;
int elapsed_t = 0;
int valido = 0;
int direction = 0;
int hor_speed = 0;
int distance = 0;
int azimute = 0;
int elevation = 0;
int mode = 0;
int marker = 0;
int status_sd = 0;
int is_open = 0;
int press = 0;
int rssid = 0;
int hdop = 9999;
int sats = 0;

String northSouth;
String eastWest;
String new_name = "";
String last_name = "test20.jpg";
String next_pkt = "";
StringList data_bar_1;


float latitude = 0.000000;
float longitude = 0.000000;
float altitude = 0;
float vert_speed = 0;
float last_alt = 0;
float last_time = 0;
float ground_speed = 0;
float bar_speed = 0;

float[] latitude_array = new float[100];
float[] longitude_array = new float[100];
float LAT = -22.708048, LON = -46.772617;

boolean follow = true;
boolean must_connect = false;
boolean connected = false;
boolean receive_image = false;
boolean receiving_image = false;
boolean received_once = false;
boolean time_out_flag = true;
Boolean stop = false;

Serial myPort;
OutputStream output;
OutputStream output2;
UnfoldingMap Mapa;
UnfoldingMap Mapa2;
Location GPS;
SimplePointMarker GPSMarker;
ControlP5 cp5;
ControlP5 cp6;
ControlP5 cp7;
ControlP5 cp8;
ControlP5 cp9;
ControlP5 cp10;
ControlP5 cp11;
ControlP5 cp12;
ControlP5 cp13;
ControlP5 cp14;
ControlP5 cp15;
ControlP5 cp16;
ControlP5 cp17;
ControlP5 cp18;
ControlP5 cp19;
ControlP5 cp20;
ControlP5 cp21;
ControlP5 cp22;
ControlP5 cp23;
ControlP5 cp24;

Chart myChart;
Chart myChart2;



public void setup() {
  latitude_array[0] = 0.0;
  longitude_array[0] = 0.0;

  size(1200, 600, P2D); // Porte de la pantalla
  if (frame != null) {
    frame.setResizable(true);
  }
  new_name = "test21.jpg";
  output = createOutput(new_name);
  output2 = createOutput(header + "log.txt");
  Mapa2 = new UnfoldingMap(this, new Microsoft.AerialProvider());
  Mapa = new UnfoldingMap(this, new Yahoo.HybridProvider());
  GPS = new Location(LAT, LON);
  GPSMarker = new SimplePointMarker(GPS);
  GPSMarker.setColor(color(255, 100, 0, 100));
  Mapa.addMarkers(GPSMarker);
  Mapa2.addMarkers(GPSMarker);
  Mapa.zoomAndPanTo(GPS, 10);
  MapUtils.createDefaultEventDispatcher(this, Mapa);
  Mapa2.zoomAndPanTo(GPS, 10);
  MapUtils.createDefaultEventDispatcher(this, Mapa2);

  data_bar_1 = new StringList();
  data_bar_1.append("Distance:");
  data_bar_1.append("Azimute:");
  data_bar_1.append("Elevation:");
  data_bar_1.append("Temp °C:");
  data_bar_1.append("Pressure:");
  data_bar_1.append("Status SD:");
  data_bar_1.append("Bar Speed:");
  data_bar_1.append("Valve");
  data_bar_1.append("Mode");
  data_bar_1.append("Elapsed T:");

  cp5 = new ControlP5(this);
  myChart = cp5.addChart("Altitude")
    .setPosition(0, 330)
      .setSize(170, 60)
        .setRange(0, 32000)
          .setView(Chart.LINE) // use Chart.LINE, Chart.PIE, Chart.AREA, Chart.BAR_CENTERED
            .setStrokeWeight(1.5)
              .setColorCaptionLabel(color(100))
                ;

  myChart.addDataSet("incoming");
  myChart.setData("incoming", new float[100]);

  cp6 = new ControlP5(this);
  myChart2 = cp6.addChart("Temperature")
    .setPosition(0, 430)
      .setSize(170, 60)
        .setRange(-50, 40)
          .setView(Chart.LINE) // use Chart.LINE, Chart.PIE, Chart.AREA, Chart.BAR_CENTERED
            .setStrokeWeight(1.5)
              .setColorCaptionLabel(color(100))
                ;

  myChart2.addDataSet("incoming");
  myChart2.setData("incoming", new float[100]);

  noStroke();

  config_bots();


  //conect();
  //GPSMarker = new SimplePointMarker(GPS);
  minim = new Minim(this);
  beep = minim.loadSnippet("beep.mp3");
  send_song = minim.loadSnippet("send.mp3");

  delay(1000);
}


//*************************************************

public void draw() {
  //delay(100);
  myChart.push("incoming", altitude);
  myChart2.push("incoming", temp_bmp);
  background(0);

  GPS.setLat(latitude);
  GPS.setLon(longitude);
  GPSMarker.setLocation(latitude, longitude);
  if (follow == true) {
    Mapa2.draw();
    Mapa.zoomAndPanTo(GPS, 1600);
    fill(#4ADB12);
    rect(990, 370, 50, 19);
  } else {
    Mapa2.draw();
    fill(#DB1212);
    rect(990, 370, 50, 19);
  }
  fill(#DB1212);
  ScreenPosition Pos = GPSMarker.getScreenPosition(Mapa2);
  balloon = loadImage("balloon.png");
  image(balloon,Pos.x - balloon.width/20, Pos.y - balloon.width/20, balloon.width/10, balloon.height/10);
  if (connected == true) {
    fill(#4ADB12);
    rect(990, 395, 50, 19);
  } else {
    fill(#DB1212);
    rect(990, 395, 50, 19);
  }
  i++;
  fill(#030303);
  rect(0, 500, 1200, 200);
  rect(1000, 0, 200, 600);
  fill(#0A10A2);
  rect(0, 550, 120, 200);
  rect(150, 550, 120, 200);
  rect(300, 550, 120, 200);
  rect(450, 550, 120, 200);
  rect(600, 550, 120, 200);
  rect(750, 550, 120, 200);
  rect(900, 550, 120, 200);
  rect(1050, 550, 220, 200);
  draw_data_bar_1();
  fill(#CE0026);
  textSize(20);
  text("FIX", 0, 570);
  text("HDOP", 150, 570);
  text("Altitude", 300, 570);
  text("Latitude", 450, 570);
  text("Longitude", 600, 570);
  text("Sats", 750, 570);
  text("Speed", 900, 570);
  text("Vertical Speed", 1050, 570);
  StringBuilder builder = new StringBuilder();
  for (String s : Serial.list ()) {
    builder.append(s);
  }
  String serial = builder.toString();
  if(valido == 1){
    fill(#00FC2C);
    text("GOOD", 0, 590);
  }else{
    text("BAD", 0, 590);
  }
  fill(#20F7C3);
  text(hdop, 150, 590);
  text(altitude, 300, 590);
  text(latitude, 450, 590);
  text(longitude, 600, 590);
  text(sats, 750, 590);
  text(ground_speed, 900, 590);
  text(vert_speed, 1050, 590);
  fill(#B7F5E5);
  textSize(20);
  //text("The Cirrus Map", 1020, 30);
  fill(#DB1212);
  rect(0, 522, 120, 20);
  /*if(receiving_image){
    time_out_flag = false;
    received_once = true;
    text("Receiving", 0, 520);
    text(received_bytes, 100, 520);
    fill(#8FFC6B);
    rect(0, 522, 120*received_bytes/file_size, 20);
  }else{
    text("Waiting", 0, 520);
  }
  */
  if(connected){
    text("RSSID", 0, 520);
    text(rssid, 70, 520);
    fill(#8FFC6B);
    rect(0, 522, 120*(150 + rssid)/150, 20);
  }
  
  if (mode == 8){
    fill(#FA0000);
    text("To Send:" + next_pkt + "!!!", 1000, 460);
  }else{
    fill(#FAE600);
    text("To Send:" + next_pkt, 1000, 460);
  }

  if ((millis() - time > 4000) && (received_once) && (time_out_flag == false)){
    stop = true;
    time_out_flag = true;
    received_bytes = 0;
    receive_image = false;
    receiving_image = false;
    println("time out");
    myPort.clear();
    last_name = new_name;
    new_name = "";
    new_counter++;
    new_name = header + new_name + new_counter + ".jpg";
    output = createOutput(new_name);
  }
  if ((received_once)){

    img = loadImage(last_name);
    if (scale > 0){
      image(img, 0, 0, img.width/2, img.height/2);
    }else{
      image(img, 0, 0, img.width, img.height);
    }
  }
}

public void controlEvent(ControlEvent theEvent) {
  println(theEvent.getController().getName());
}

//******************************************************************************

void Follow(int theValue) {
  mode = 122;
  println("a button event from colorA: ");
  if (follow == true) {
    follow = false;
  } else {
    follow = true;
  }
}

//******************************************************************************

void Connect(int theValue) {
  if (must_connect) {
    conect();
  }
  must_connect = true;
  println("connect");
}

//******************************************************************************

void Image_size(int theValue) {
  scale = scale*(-1);
}

//******************************************************************************

void conect() {
  println(Serial.list());
  myPort = new Serial(this, "COM1", 9600);
  connected = true;
}
//******************************************************************************

void Auto(int theValue) {
  mode = 0;
  marker = 20;
  send_base("auto\n");
}
//******************************************************************************

void Photo(int theValue) {
  mode = 1;
  marker = 20+25*mode;
  send_base("send_photo\n");
}
//******************************************************************************

void Medium(int theValue) {
  mode = 3;
  marker = 20+25*(mode - 1);
  send_base("medium_stab\n");
}
//******************************************************************************

void Full(int theValue) {
  mode = 4;
  marker = 20+25*(mode - 1);
  send_base("full_stab\n");
}
//******************************************************************************

void Custom(int theValue) {
  mode = 5;
  marker = 20+25*(mode - 1);
  send_base("custom_stab\n");
}
//******************************************************************************

void Open(int theValue) {
  mode = 6;
  marker = 20+25*(mode - 1);
  send_base("open_valve\n");
}
//******************************************************************************

void Close(int theValue) {
  mode = 7;
  marker = 20+25*(mode - 1);
  send_base("close_valve\n");
}
//******************************************************************************

void End_Flight(int theValue) {
  mode = 8;
  marker = 20+25*(mode - 1);
  send_base("end_flight\n");
}
//******************************************************************************

void Bounce(int theValue) {
  mode = 9;
  marker = 20+25*(mode - 1);
  send_base("bounce\n");
}
//******************************************************************************

void Load_String(int theValue) {
  mode = 10;
  marker = 20+25*(mode - 1);
  send_base("load_string\n");
}
//******************************************************************************

void Reset(int theValue) {
  mode = 11;
  marker = 20+25*(mode - 1);
  send_base("reset\n");
}
//******************************************************************************

void Delay_M15(int theValue) {
  mode = 12;
  marker = 20+25*(mode - 1);
  send_base("delay_15\n");
}
//******************************************************************************

void Delay_M30(int theValue) {
  mode = 13;
  marker = 20+25*(mode - 1);
  send_base("delay_30\n");
}
//******************************************************************************

void Delay_M60(int theValue) {
  mode = 14;
  marker = 20+25*(mode - 1);
  send_base("delay_60\n");
}
//******************************************************************************

void SEND(int theValue) {
  if(connected){
    myPort.write(next_pkt);
  }
  println("SENDING");
  send_song.play();
  send_song.rewind();

}

//******************************************************************************

void send_base(String pkt){
  next_pkt = pkt;
  print("will_send");
}
//******************************************************************************

void serialEvent(Serial myPort) {
  beep.play();
  beep.rewind();
  if(receive_image){
    try {
    if(myPort.available () > 0 ){
      time = millis();
      receiving_image = true;
    }
    while ( myPort.available () > 0 ) {
      output.write(myPort.read());
      received_bytes++;
      delay(1);
    }
  }
  catch (IOException e) {
    e.printStackTrace();
  }
  }else{
    
    String data  =   (myPort.readStringUntil ( 'y' ) ) ;
    println(data);
    if(data != null){
      parseString(data);
      data = "";
    }
  }


  }

//******************************************************************************
void parseString(String serialString) {
  myPort.clear();
  //String Padrão
  //(CIR;Valido;lat;lon;HDOP;Sats;alt_gps;direção;vel_kph;temp_bmp;alt_bmp)
  //2;1;-220000;-470000;9999;0;836;0;3;3.43;89.85;-1;0;30355;235;1;0;0;0;458;x

  println(serialString);

  String items[] = (split(serialString, ';'));
  println((items[0]));
  float rx_mode = (float(items[0]));
  println(rx_mode);
  if (rx_mode == 2.0) {
    println("chegou");
    valido = int(items[1]);
    latitude = float(items[2])/10000;
    longitude = float(items[3])/10000;
    hdop = int(items[4]);
    sats = int(items[5]);
    last_alt = altitude;
    altitude = int(items[6]);
    vert_speed = (last_alt - altitude)/((millis() - last_time)/1000);
    last_time = millis();
    direction = int(items[7]);
    hor_speed = int(items[8]);
    distance = int(items[9]);
    azimute = int(items[10]);
    elevation = int(items[11]);
    temp_bmp = int(items[12]);
    alt_bmp = int(items[13]);
    press = int(items[14]);
    status_sd = int(items[15]);
    bar_speed = float(items[16]);
    is_open = int(items[17]);
    mode = int(items[18]);
    elapsed_t = int(items[19]);
    rssid = int(items[21]);
    beep.play();
    
    /*
    string_data.concat(gps_lat * 10000); string_data.concat(";");
    string_data.concat(gps_lng * 10000); string_data.concat(";");
    string_data.concat(gps_hdop_value); string_data.concat(";");
    string_data.concat(gps_satellites_value); string_data.concat(";");
    string_data.concat(gps_altitude_meters); string_data.concat(";");
    7
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
    string_data.concat(mode); ; string_data.concat(";");
    string_data.concat(millis() / 1000);
    */
  }else if(rx_mode == 1.0){
    println("got an image");
    file_size = int(items[1]);
    packet_number = int(items[2]);
    print("size: "); print(file_size); print("packets: "); println(packet_number);
    receive_image = true;
  }

}
float convert(float to_conv) {
  int first = ((int)to_conv)/100;
  float next = to_conv - (float)first*100;
  float final_anw = (float)(first + next/60.0);
  return -1*final_anw;
}

//******************************************************************************
          /*
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
          */

void config_bots(){

  cp7 = new ControlP5(this);

  cp7.addButton("Follow")
    .setValue(0)
      .setPosition(1000, 370)
        .setSize(200, 19)
          ;

  cp8 = new ControlP5(this);

  cp8.addButton("Connect")
    .setValue(0)
      .setPosition(1000, 395)
        .setSize(200, 19)
          ;

  cp9 = new ControlP5(this);
  cp9.addButton("Image_size")
    .setValue(0)
      .setPosition(1000, 420)
        .setSize(200, 19)
          ;

  cp10 = new ControlP5(this);

  cp10.addButton("Auto")
    .setValue(0)
      .setPosition(1000, 20)
        .setSize(200, 19)
          ;

  cp11 = new ControlP5(this);

  cp11.addButton("Send Photo")
    .setValue(0)
      .setPosition(1000, 45)
        .setSize(200, 19)
          ;

  cp12 = new ControlP5(this);
  cp12.addButton("Medium")
    .setValue(0)
      .setPosition(1000, 70)
        .setSize(200, 19)
          ;
  cp13 = new ControlP5(this);

  cp13.addButton("Full")
    .setValue(0)
      .setPosition(1000, 95)
        .setSize(200, 19)
          ;

  cp14 = new ControlP5(this);

  cp14.addButton("Custom")
    .setValue(0)
      .setPosition(1000, 120)
        .setSize(200, 19)
          ;

  cp15 = new ControlP5(this);
  cp15.addButton("Open")
    .setValue(0)
      .setPosition(1000, 145)
        .setSize(200, 19)
          ;

  cp16 = new ControlP5(this);

  cp16.addButton("Close")
    .setValue(0)
      .setPosition(1000, 170)
        .setSize(200, 19)
          ;

  cp17 = new ControlP5(this);

  cp17.addButton("End_Flight")
    .setValue(0)
      .setPosition(1000, 195)
        .setSize(200, 19)
          ;

  cp18 = new ControlP5(this);
  cp18.addButton("Bounce")
    .setValue(0)
      .setPosition(1000, 220)
        .setSize(200, 19)
          ;
  cp19 = new ControlP5(this);

  cp19.addButton("Load_String")
    .setValue(0)
      .setPosition(1000, 245)
        .setSize(200, 19)
          ;

  cp20 = new ControlP5(this);

  cp20.addButton("Reset")
    .setValue(0)
      .setPosition(1000, 270)
        .setSize(200, 19)
          ;

  cp21 = new ControlP5(this);
  cp21.addButton("Delay_M15")
    .setValue(0)
      .setPosition(1000, 295)
        .setSize(200, 19)
          ;
  cp22 = new ControlP5(this);

  cp22.addButton("Delay_M30")
    .setValue(0)
      .setPosition(1000, 320)
        .setSize(200, 19)
          ;

  cp23 = new ControlP5(this);

  cp23.addButton("Delay_M60")
    .setValue(0)
      .setPosition(1000, 345)
        .setSize(200, 19)
          ;

  cp24 = new ControlP5(this);

  cp24.addButton("SEND")
    .setValue(0)
      .setPosition(1000, 470)
        .setSize(200, 19)
          ;
  cp24.setColorBackground(color(#0B7C04)); 
}

void draw_data_bar_1(){
  fill(#000000);
  rect(0, 0, 170, 500);
  fill(#0A10A2);
  for (int i = 20; i < 300; i = i+25) {
    rect(0, i, 170, 19);
}
  fill(#FFFFFF);
  for (int i = 1; i < 10; i++) {
    textSize(16);
    String item = data_bar_1.get(i);
    text (item, 0, 12 + i*25);
}
  text ("Distance:", 0, 12 + 10*25);
  text ("Alt Bar.:", 0, 12 + 11*25);
  text ("Direction:", 0, 12 + 12*25);
  fill(#F4F500);
  text(azimute, 100, 12+25);
  text(elevation, 100, 12+2*25);
  text(temp_bmp, 100, 12+3*25);
  text(press, 100, 12+4*25);
  text(bar_speed, 100, 12+6*25);
  text(mode, 100, 12+8*25);
  text(elapsed_t, 100, 12+9*25);
  text(distance, 100, 12+10*25);
  text(alt_bmp, 100, 12+11*25);
  text(direction, 100, 12+12*25);
  if(status_sd == 1){
    fill(#1BF500);
    text("GOOD", 100, 12+5*25);
  }else{
    fill(#FF0004);
    text("BAD", 100, 12+5*25);
  }
  if(is_open == 1){
    fill(#1BF500);
    text("OPEN", 100, 12+7*25);
  }else{
    fill(#FF0004);
    text("CLOSED", 100, 12+7*25);
  }
}

