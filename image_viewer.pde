// implementar https://www.tigoe.com/pcomp/code/Processing/23/
import processing.serial.*; 
import controlP5.*;
import processing.core.PApplet;
import de.fhpotsdam.unfolding.UnfoldingMap;
import de.fhpotsdam.unfolding.geo.Location;
import de.fhpotsdam.unfolding.utils.MapUtils;
import de.fhpotsdam.unfolding.marker.SimplePointMarker;
import de.fhpotsdam.unfolding.providers.OpenStreetMap.*;
//import de.fhpotsdam.unfolding.providers.PrecipitationClassic.*;
import de.fhpotsdam.unfolding.providers.Microsoft;
import de.fhpotsdam.unfolding.providers.Yahoo;
//import de.fhpotsdam.unfolding.providers.OpenWeatherProvider;
import processing.opengl.*;
import saito.objloader.*;
import processing.serial.*;

PImage img;

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

String northSouth;   
String eastWest; 


float latitude = 0.000000; 
float longitude = 0.000000;
float hdop = 9999;
float sats = 0;
float altitude = 0;
float vert_speed = 0;
float last_alt = 0;
float last_time = 0;
float ground_speed = 0;


boolean follow = true;
boolean must_connect = false;
boolean connected = false;
boolean valido = false;
boolean receive_image = false;
boolean receiving_image = false;
boolean received_once = false;
boolean time_out_flag = true;
Boolean stop = false;



Serial myPort;
OutputStream output;
UnfoldingMap Mapa;     
UnfoldingMap Mapa2;
Location GPS;                
SimplePointMarker GPSMarker; 
ControlP5 cp5;
ControlP5 cp6;
ControlP5 cp7;
ControlP5 cp8;
ControlP5 cp9;
Chart myChart;
Chart myChart2;

float LAT = -22.708048, LON = -46.772617;  

public void setup() {

  size(1200, 600, OPENGL); // Porte de la pantalla
  if (frame != null) { 
    frame.setResizable(true);
  } 
  output = createOutput("test20.jpg");
  // Configuracion del mapa
  //OpenWeatherProvider.PrecipitationClassic()
  //Mapa = new UnfoldingMap(this, new OpenStreetMapProvider());
  //Mapa = new UnfoldingMap(this, new OpenWeatherProvider());
  Mapa2 = new UnfoldingMap(this, new Microsoft.AerialProvider());
  Mapa = new UnfoldingMap(this, new Yahoo.HybridProvider());
  //Mapa = new UnfoldingMap(this, new OpenWeatherProvider.Wind());
  GPS = new Location(LAT, LON);                                

  GPSMarker = new SimplePointMarker(GPS); 
  //connectionMarker = new SimpleLinesMarker(Location, Location);
  GPSMarker.setColor(color(255, 100, 0, 100));
  Mapa.addMarkers(GPSMarker);   
  Mapa2.addMarkers(GPSMarker);  

  Mapa.zoomAndPanTo(GPS, 10);                     
  MapUtils.createDefaultEventDispatcher(this, Mapa); 
  
  Mapa2.zoomAndPanTo(GPS, 10);                     
  MapUtils.createDefaultEventDispatcher(this, Mapa2); 
  
  cp5 = new ControlP5(this);
  myChart = cp5.addChart("Altitude")
    .setPosition(1000, 50)
      .setSize(200, 100)
        .setRange(0, 32000)
          .setView(Chart.LINE) // use Chart.LINE, Chart.PIE, Chart.AREA, Chart.BAR_CENTERED
            .setStrokeWeight(1.5)
              .setColorCaptionLabel(color(100))
                ;

  myChart.addDataSet("incoming");
  myChart.setData("incoming", new float[100]);

  cp6 = new ControlP5(this);
  myChart2 = cp6.addChart("Temperature")
    .setPosition(1000, 180)
      .setSize(200, 100)
        .setRange(-50, 40)
          .setView(Chart.LINE) // use Chart.LINE, Chart.PIE, Chart.AREA, Chart.BAR_CENTERED
            .setStrokeWeight(1.5)
              .setColorCaptionLabel(color(100))
                ;

  myChart2.addDataSet("incoming");
  myChart2.setData("incoming", new float[100]);

  noStroke();
  cp7 = new ControlP5(this);

  cp7.addButton("Follow")
    .setValue(0)
      .setPosition(1000, 320)
        .setSize(200, 19)
          ;

  cp8 = new ControlP5(this);

  cp8.addButton("Connect")
    .setValue(0)
      .setPosition(1000, 345)
        .setSize(200, 19)
          ;

  cp9 = new ControlP5(this);
  cp9.addButton("I")
    .setValue(0)
      .setPosition(1000, 370)
        .setSize(200, 19)
          ;
  //conect();

  delay(1000);
}

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
    rect(990, 320, 50, 19);
    
  } else {
    Mapa2.draw();
    fill(#DB1212);
    rect(990, 320, 50, 19);
    
  } 
  if (connected == true) {
    fill(#4ADB12);
    rect(990, 345, 50, 19);
  } else {
    fill(#DB1212);
    rect(990, 345, 50, 19);
  }

  i++;
  //delay(10);
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
  
  rect(1000, 400, 200, 19);
  rect(1000, 425, 200, 19);
  rect(1000, 450, 200, 19);
  
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
  text("Temp.", 1010, 415);
  text("Alt Bar.", 1010, 440);
  text("Pressure.", 1010, 465);
  StringBuilder builder = new StringBuilder();
  for (String s : Serial.list ()) {
    builder.append(s);
  }
  String serial = builder.toString();
  //println(serial);
  if(valido){
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
  
  text(temp_bmp, 1100, 415);
  text(alt_bmp, 1100, 440);
  text("NONE", 1100, 465);
  fill(#DB1212);
  textSize(20);
  text("The Cirrus Map", 1020, 30);
  rect(0, 522, 120, 20);
  if(receiving_image){
    time_out_flag = false;
    received_once = true;
    text("Receiving", 0, 520);
    text(received_bytes, 100, 520);
    text(100*received_bytes/file_size, 200, 600);
    fill(#8FFC6B);
    rect(0, 522, 120*received_bytes/file_size, 20);
    //println("receiving");
  }else{
    text("Waiting", 0, 520);
  }

  if ((millis() - time > 10000) && (received_once) && (time_out_flag == false)){
    stop = true;
    //println("finished");
    time_out_flag = true;
    received_bytes = 0;
    receive_image = false;
    //img = loadImage("test20.jpg");
    //image(img, 0, 0);
    receiving_image = false;
    println("time out");
    myPort.clear();
  }
  if ((received_once)){
    img = loadImage("test20.jpg");
    image(img, 0, 0, img.width/2, img.height/2);
  }
  /*String data  =   (myPort.readStringUntil ( 'x' ) ) ; 
  if (data != null){
    print(data);
    parseString(data);
  }
  */
}

public void controlEvent(ControlEvent theEvent) {
  println(theEvent.getController().getName());
}

void Follow(int theValue) {
  println("a button event from colorA: ");
  if (follow == true) {
    follow = false;
  } else {
    follow = true;
  }
}

void Connect(int theValue) {
  if (must_connect) {
    conect();
  }
  must_connect = true;
  println("connect");
}

void I(int theValue) {
  //receive_image = true;
  println("image");
}

void conect() {
  println(Serial.list());
  myPort = new Serial(this, "COM8", 115200);
  //myPort.bufferUntil('x');
  connected = true;
}


void serialEvent(Serial myPort) { 
  /*String myString = myPort.readStringUntil(linefeed);
  if (myString != null) {
    parseString(myString);
  }*/
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
    String data  =   (myPort.readStringUntil ( 'x' ) ) ; 
    if(data != null){
      //println(data);
      parseString(data);
      data = "";
    }
  }
  
  /*if(receive_image==false){
    try { 
      String data = myPort.readStringUntil ( 'x' );
      print(data);
      parseString(data); 
  }finally{
  }*/
  /*}else{
    String data  =   (myPort.readStringUntil ( 'x' ) ) ; 
    print(data);
    parseString(data);
*/
    
  }
 


void parseString(String serialString) {
  myPort.clear();
  //String Padrão
  //(CIR;Valido;lat;lon;HDOP;Sats;alt_gps;direção;vel_kph;temp_bmp;alt_bmp)
  println(serialString);
  
  String items[] = (split(serialString, ';'));
  if (items[0].equals("CIR") == true) {
    println("chegou");
    valido = boolean(items[2]);
    latitude = float(items[2]);
    longitude = float(items[3]);
    hdop = float(items[5]);
    sats = float(items[6]);
    last_alt = altitude;
    altitude = float(items[7]);
    vert_speed = (last_alt - altitude)/((millis() - last_time)/1000);
    last_time = millis();
    //dir = int(items[8]);
    //ground_speed = float(items[9]);
    //temp_bmp = int(items[10]);
    //alt_bmp = int(items[11]);
    //println(latitude + northSouth + "," +longitude + eastWest + "," + vert_speed);
  }else if(items[0].equals("img") == true){
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

