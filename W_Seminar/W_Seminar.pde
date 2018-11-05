//W Seminar App


//Import Sensor stuff
import ketai.sensors.*;
//import Bluetooth stuff
import android.content.Intent;
import android.os.Bundle;
import ketai.net.*;
import ketai.net.bluetooth.*;
import ketai.ui.*;

PImage[] img = new PImage[35];  
PFont pixel;

Button houseButton;
Button controllerButton;
Button backButton;
Button backControlButton;
Button houseBlueButton;
Button houseYellowButton;
Button houseGreenButton;
Button houseRedButton;
Button retryButton;
Button textStartButton;
Button textZielButton;
Button turnLeftButton;
Button turnRightButton;
Button leftButton;
Button rightButton;
Button armDownButton;
Button armUpButton;
Slider forewardSlider;
Kompass carCircle;

int page = 0; // 0 = Menu, 1 = House, 2 = Controller
int oldPage = 0; 
int eingabe = 0;
float rotation;
int statusNum;
float distFront = 10;
float distBack = 100;
float distFrontOld;
float distBackOld;
float maxDistance = 200;
boolean armDown = false;
boolean pressed = false;
int activeButton = 0;    //0=no Button, 1=Left, 2=Right, 3=TurnLeft, 4=TurnRight, 5=ArmDown, 6=ArmUp
String start = "";
String ziel = "";
String status = "";
String oldStart = "";
String oldZiel = "";
String oldStatus = "";

//Bluetooth stuff:
KetaiBluetooth bt;
boolean isConfiguring = true;
boolean oldConnection = false;
String info = "";
KetaiList klist;
ArrayList devicesDiscovered = new ArrayList();
byte[] datIn = new byte[6];
byte[] bleInput = new byte [6];
int charCount = 0; 
long bleTime;
int sendDelay = 50;

//enable bluetooth on startup
void onCreate(Bundle savedInstanceState) {
  super.onCreate(savedInstanceState);
  bt = new KetaiBluetooth(this);
}

void onActivityResult(int requestCode, int resultCode, Intent data) {
  bt.onActivityResult(requestCode, resultCode, data);
}

//Sensor stuff:
KetaiSensor sensor;
int accelerometerY;


void setup() {                                //on start
  orientation(PORTRAIT);
  fullScreen();
  noStroke();
  imageMode(CENTER);
  noSmooth();
  pixel = loadFont("gameovercre-64.vlw");
  textFont(pixel);

  //Bluetooth:
  //start listening for Connections
  bt.start();
  isConfiguring = true;

  //Sensor stuff:
  sensor = new KetaiSensor(this);
  sensor.start();

  loadImages();
  loadButtons();
  loadMainMenu();
}

void draw() {                                //loop
  if (page!=oldPage) {
    if (page == 1) { 
      println("HouseMode");
      loadHouseMode();
      setEingabe("");
    } else if (page == 2) {
      loadControlMode();
      println("ControlMode");
    } else {
      loadMainMenu();
      println("MainMenu");
    }
    oldPage = page;
  }
  if (connected() != oldConnection) {
    println("connection: " + connected());
    checkConnectionLED();
    oldConnection = connected();
  }
  if (page == 1 && (start != oldStart || ziel != oldZiel || status != oldStatus)) {
    fill(192, 234, 203);
    rect(350, 1400, width-400, 600);
    fill(0);
    textAlign(LEFT, CENTER);
    text(start, 400, 1500);
    text(ziel, 400, 1650);
    text(status, 400, 1800);
    oldStart = start;  
    oldZiel = ziel; 
    oldStatus = status;
  }
  forewardSlider.slide();
  //if (forewardSlider.value() != 0)
  //  println(forewardSlider.value());
  if (page==2) {
    carCircle.update();
    if (millis()>=bleTime && bleTime!=0) {
      sendData(0, 0);
      bleTime = 0;
    }
  }
  if ((page == 2 && (distFront != distFrontOld || distBack != distBackOld)) || status == "nicht verbunden!" && page == 2) {
    fill(239, 231, 202);
    rect(30, 1220, 250, 570);
    fill(0);
    textAlign(LEFT, CENTER);
    pushMatrix();
    translate(90, 1220);
    rotate(radians(90));
    if (status == "nicht verbunden!") {
      text(status, 0, -130);
      text(status, 0, 0);
    } else {
      if (distFront < maxDistance && (int)distFront != 0) {
        text((int)distFront, 0, -130);
        text("cm", 150, -130);
      } else {
        text("zu weit!", 0, -130);
      }
      if (distBack < maxDistance && (int)distBack != 0) {
        text((int)distBack, 0, 0);
        text("cm", 150, 0);
      } else {
        text("zu weit!", 0, 0);
      }
    }
    popMatrix();
    distFrontOld = distFront;
    distBackOld = distBack;
  }

  //Bluetooth:
  if (isConfiguring) {
    ArrayList names; 
    //background(78, 93, 75);
    klist = new KetaiList(this, bt.getPairedDeviceNames());
    println("bluettoth configured!");
    isConfiguring = false;
  }
}

void mousePressed() {
  pressed = true;
  if (houseButton.mouseIsOver()) {  
    page = 1;
    return;
  } 
  if (controllerButton.mouseIsOver()) {
    page = 2;
    return;
  }
  if (backButton.mouseIsOver() || backControlButton.mouseIsOver()) {
    page = 0;
    return;
  }
  if (textStartButton.mouseIsOver()) {
    setEingabe("");
    return;
  }
  if (textZielButton.mouseIsOver()) {
    setEingabe("");
    return;
  }
  if (houseBlueButton.mouseIsOver()) {
    println("blaues Haus");
    if (eingabe>0)
      setEingabe("blaues Haus");
    return;
  }
  if (houseYellowButton.mouseIsOver()) {
    println("gelbes Haus");
    if (eingabe>0)
      setEingabe("gelbes Haus");
    return;
  }
  if (houseGreenButton.mouseIsOver()) {
    println("gruenes Haus");
    if (eingabe>0)
      setEingabe("gruenes Haus");
    return;
  }
  if (houseRedButton.mouseIsOver()) {
    println("rotes Haus");
    if (eingabe>0)
      setEingabe("rotes Haus");
    return;
  }
  if (retryButton.mouseIsOver()) {
    setEingabe("");
    return;
  }
  if (turnLeftButton.mouseIsOver()) {
    println("turn left!");
    activeButton = 3;
    return;
  }
  if (turnRightButton.mouseIsOver()) {
    println("turn right!");
    activeButton = 4;
    return;
  }
  if (leftButton.mouseIsOver()) {
    println("left!");
    activeButton = 1;
    return;
  }
  if (rightButton.mouseIsOver()) {
    println("right!");
    activeButton = 2;
    return;
  }
  if (armDownButton.mouseIsOver()) {
    println("arm Down!");
    armDown = true;
    activeButton = 5;
    return;
  }
  if (armUpButton.mouseIsOver()) {
    println("arm Up!");
    armDown = false;
    activeButton = 6;
    return;
  }
  if (forewardSlider.mouseIsOver()) {
    forewardSlider.touched = true;
    return;
  }
}

void mouseReleased() {
  pressed = false;
  activeButton = 0;
}


void loadMainMenu() {
  clearVariables();
  background(199, 229, 226);
  deactivateButtons();            //alte Buttons deaktivieren
  checkConnectionLED();
  houseButton.drawButton();
  controllerButton.drawButton();
  houseButton.activate();
  controllerButton.activate();
}

void loadHouseMode() {
  background(192, 234, 203);
  deactivateButtons();            //alte Buttons deaktivieren
  image(img[3], width/2, 650, (int)15*img[3].width, (int)15*img[3].height);  //display Strecke
  oldStart = "";  
  oldZiel = ""; 
  oldStatus = "";
  checkConnectionLED();
  textAlign(LEFT, CENTER);
  text("Start:", 70, 1500);
  text("Ziel:", 70, 1650);
  text("Status:", 70, 1800);
  backButton.drawButton();
  houseBlueButton.drawButton();
  houseYellowButton.drawButton();
  houseGreenButton.drawButton();
  houseRedButton.drawButton();
  retryButton.drawButton();
  backButton.activate();
  houseBlueButton.activate();
  houseYellowButton.activate();
  houseGreenButton.activate();
  houseRedButton.activate();
  if (eingabe==0)
    retryButton.activate();
}

void loadControlMode() {
  background(239, 231, 202);
  deactivateButtons();            //alte Buttons deaktivieren
  image(img[18], 220, 1100, (int)15*img[18].width, (int)15*img[18].height); //display distance Front Symbol
  image(img[19], 90, 1100, (int)15*img[18].width, (int)15*img[18].height); //display distance Back Symbol
  //distFront = 999;
  //distBack = 100;
  checkConnectionLED();
  backControlButton.drawButton();
  turnLeftButton.drawButton();
  turnRightButton.drawButton();
  leftButton.drawButton();
  rightButton.drawButton();
  armDownButton.drawButton();
  armUpButton.drawButton();
  backControlButton.activate();
  turnLeftButton.activate();
  turnRightButton.activate();
  leftButton.activate();
  rightButton.activate();
  armDownButton.activate();
  armUpButton.activate();
}

void setEingabe(String message) {               // Logik Zieleingabe 
  if (eingabe == 4 && start != message) {
    ziel = message;
    //houseToNum: ""->0,"blaues Haus"->1,"gelbes Haus"->2,"gruenes Haus"->3,"rotes Haus"->4
    sendData(houseToNum(start), houseToNum(ziel));
    eingabe = 0;
    loadHouseMode();
  }
  if (eingabe == 3) {
    eingabe = 4;
    loadHouseMode();
  }
  if (eingabe == 2) {
    deactivateButtons();
    start = message;
    textZielButton.drawButton();
    textZielButton.activate();
    eingabe = 3;
  }
  if (eingabe == 1) {
    eingabe = 2;
    loadHouseMode();
  }
  if (eingabe==0 && message == "") {
    deactivateButtons();
    start = "";
    ziel = "";
    eingabe = 1;
    textStartButton.drawButton();
    textStartButton.activate();
  }
}


void loadImages() {
  img[0] = loadImage("Haus.png");
  img[1] = loadImage("Controller.png");
  img[2] = loadImage("Zurueck.png");
  img[3] = loadImage("Strecke.png");
  img[4] = loadImage("Haus_blau.png");
  img[5] = loadImage("Haus_gelb.png");
  img[6] = loadImage("Haus_gruen.png");
  img[7] = loadImage("Haus_rot.png");
  img[8] = loadImage("LEDRed.png");
  img[9] = loadImage("LEDGreen.png");
  img[10] = loadImage("Wiederholen.png");
  img[11] = loadImage("Textbox.png");
  img[12] = loadImage("slider.png");
  img[13] = loadImage("sliderKnob.png");
  img[14] = loadImage("left.png");
  img[15] = loadImage("right.png");
  img[16] = loadImage("turnLeft.png");
  img[17] = loadImage("turnRight.png");
  img[18] = loadImage("distanceFront.png");
  img[19] = loadImage("distanceBack.png");
  img[20] = loadImage("Auto_blank.png");
  img[21] = loadImage("Auto_links.png");
  img[22] = loadImage("Auto_rechts.png");
  img[23] = loadImage("Auto_beide.png");
  img[24] = loadImage("Arm_blank.png");
  img[25] = loadImage("Arm_links.png");
  img[26] = loadImage("Arm_mitte.png");
  img[27] = loadImage("Arm_rechts.png");
  img[28] = loadImage("Arm_links_mitte.png");
  img[29] = loadImage("Arm_links_rechts.png");
  img[30] = loadImage("Arm_mitte_rechts.png");
  img[31] = loadImage("Arm_komplett.png");
  img[32] = loadImage("Zurueck_rotate.png");
  img[33] = loadImage("armDown.png");
  img[34] = loadImage("armUp.png");
}

void loadButtons() {
  forewardSlider = new Slider(img[13], img[12], width/2, 150, (width/2)-150, 15, 15);
  carCircle = new Kompass(img[20], img[24], width-410, height-430, 730, 247, 7, 15);
  houseButton = new Button("", img[0], width/2, 600, 20);
  controllerButton = new Button("", img[1], width/2, 1200, 20);
  backButton = new Button ("", img[2], 90, 90, 15);
  backControlButton = new Button ("", img[32], width-90, height-90, 15);
  houseBlueButton = new Button("", img[4], (width/2)-195, 432, 15);
  houseYellowButton = new Button("", img[5], (width/2)+195, 432, 15);
  houseGreenButton = new Button("", img[6], (width/2)-195, 822, 15);
  houseRedButton = new Button("", img[7], (width/2)+195, 822, 15);
  retryButton = new Button("", img[10], 170, 1300, 15);
  textStartButton = new Button("Wo sind sie gerade?", img[11], width/2, 650, 15);
  textZielButton = new Button("Wo moechten sie hin?", img[11], width/2, 650, 15);
  turnLeftButton = new Button("", img[16], width-width/4, 490, 15);
  turnRightButton = new Button("", img[17], width-width/4, 890, 15);
  leftButton = new Button("", img[14], width/2, 490, 15);
  rightButton = new Button("", img[15], width/2, 890, 15);
  armDownButton = new Button("", img[33], width/4, 550, 15);
  armUpButton = new Button("", img[34], width/4, 830, 15);
}

void deactivateButtons() {
  houseButton.deactivate();
  controllerButton.deactivate();
  backButton.deactivate();
  controllerButton.deactivate();
  houseBlueButton.deactivate();
  houseYellowButton.deactivate();
  houseGreenButton.deactivate();
  houseRedButton.deactivate();
  retryButton.deactivate();
  textStartButton.deactivate();
  textZielButton.deactivate();
  turnLeftButton.deactivate();
  turnRightButton.deactivate();
  leftButton.deactivate();
  rightButton.deactivate();
  armDownButton.deactivate();
  armUpButton.deactivate();
}

void clearVariables() {
  eingabe = 0;
  pressed = false;
  start = "";
  ziel = "";
  status = "";
  oldStart = "";
  oldZiel = "";
  oldStatus = "";
}

void checkConnectionLED() {
  if (connected()) {
    if (page == 2) {
      image(img[9], 90, height-85, (int)15*img[9].width, (int)15*img[9].height);
    } else {
      image(img[9], width-90, 85, (int)15*img[9].width, (int)15*img[9].height);
    }
  } else {
    if (page == 2) {
      image(img[8], 90, height-85, (int)15*img[8].width, (int)15*img[8].height);
    } else {
      image(img[8], width-90, 85, (int)15*img[8].width, (int)15*img[8].height);
      status = "nicht verbunden!";
    }
  }
}

int houseToNum(String houseString) {
  if (houseString == "blaues Haus")
    return 1;
  if (houseString == "gelbes Haus")
    return 2;
  if (houseString == "gruenes Haus")
    return 3;
  if (houseString == "rotes Haus")
    return 4;
  return 0;
}

void statusToString() {
  switch(statusNum) {
  case 0: 
    status = "Kalibrierung"; 
    break;
  case 1: 
    status = "bereit"; 
    break;
  case 2: 
    status = "fahre zu Start"; 
    break;
  case 3: 
    status = "bin bei Start"; 
    break;
  case 4: 
    status = "fahre zu Ziel"; 
    break;
  case 5: 
    status = "bin bei Ziel"; 
    break;
  case 6: 
    status = "fahre zurück"; 
    break;
  default: 
    status = "Kommunikationsfehler";
    break;
  }
}


//Bluetooth
void sendData(int fromHouse, int toHouse) {
  byte[] data = {(byte)fromHouse, '|', (byte)toHouse, '|', (byte)activeButton, '|', (byte)(forewardSlider.value()+100), '|', (byte)(accelerometerY+100), 'E'};
  if (!isConfiguring && connected()) {
    bt.broadcast(data);
  }
}

void onKetaiListSelection(KetaiList klist) {
  String selection = klist.getSelection();
  bt.connectToDeviceByName(selection);
  klist = null;
}


//Call back method to manage data received
void onBluetoothDataEvent(String who, byte[] data) {
  if (isConfiguring && !connected())
    return;
  info += new String(data);
  if (info.contains("E")) {
    info = info.substring(0, info.length()-1);
    processInput();
    info = "";
  }
}

boolean connected() {
  for (String st : bt.getConnectedDeviceNames()) {
    if (st.equals("HC-06")) {
      return true;
    }
  }
  return false;
}

void processInput() {


  String[] vals = split(info, '|');
  rotation = parseInt(vals[0]);
  statusNum = parseInt(vals[1]);
  distFront = parseInt(vals[2]);
  distBack = parseInt(vals[3]);
  statusToString();
  bleTime = millis() + sendDelay;
  print("receiving:  " + rotation + "  " + status + "  " + distFront + "  " + distBack);
}

//Sensor:
void onAccelerometerEvent(float x, float y, float z)
{
  accelerometerY = (int)map(constrain(y, -6, 6), -6, 6, -100, 100);
  if (accelerometerY < 20 &&accelerometerY > -20)
    accelerometerY = 0;
}

class Button {
  boolean activeButton = false;
  PImage imageButton; 
  String label; // button label
  float x;      // top left corner x position
  float y;      // top left corner y position
  float s;      // scale factor

  // constructor
  Button(String labelB, PImage imageB, float xpos, float ypos, float scaleB) {
    label = labelB;
    imageButton = imageB;
    x = xpos;
    y = ypos;
    s = scaleB;
  }

  void drawButton() {
    image(imageButton, x, y, (int)s*imageButton.width, (int)s*imageButton.height);
    textAlign(CENTER, CENTER);
    fill(0);
    text(label, x + (imageButton.width / 2), y + (imageButton.height / 2));
  }

  void activate() {
    activeButton = true;
  }

  void deactivate() {
    activeButton = false;
  }

  boolean mouseIsOver() {
    if (activeButton) {
      if (mouseX > (x - (imageButton.width*s/2)) && mouseX < (x + (imageButton.width*s/2)) && mouseY > (y - (imageButton.height*s/2)) && mouseY < (y + (imageButton.height*s/2))) {
        return true;
      }
    }
    return false;
  }
}

class Slider {
  boolean touched = false;
  float imgX;  //Knob Position X
  PImage imageSliderKnob;
  PImage imageSlider;
  float x;    //X neutral Position center
  float y;    //Y neutral Position center
  float d;    //ditance one way
  int s;      //speed Knob
  int scale;  //scale Knob


  Slider(PImage imageSKnob, PImage imageS, float xpos, float ypos, float distance, int speed, int scaleS) {
    imageSliderKnob = imageSKnob;
    imageSlider = imageS;
    x = xpos;
    y = ypos;
    d = distance;
    s = speed;
    imgX = xpos;
    scale = scaleS;
  }

  void slide() {
    if (!pressed)
      touched = false;
    if (page == 2) {
      float difx;
      if (touched) {               //dem Finger nach!
        difx = mouseX - imgX;
      } else {                     //zurück in die Mitte!
        difx = x - imgX;
      }
      if (abs(difx)>1) {
        imgX = imgX + difx/s;
        imgX = constrain(imgX, x-d, x+d);
      }
      fill(239, 231, 202);
      rect(0, 0, width, 300);
      image(imageSlider, x, y, (int)scale*imageSlider.width, (int)scale*imageSlider.height);
      image(imageSliderKnob, imgX, y, (int)scale*imageSliderKnob.width, (int)scale*imageSliderKnob.height);
    }
  }

  int value() {
    int val = (int)map(imgX, x-d, x+d, -100, 100);
    if (val < 5 && val > -5) {
      return(0);
    } else {
      return val;
    }
  }

  boolean mouseIsOver() {
    if (mouseX > (x - (imageSliderKnob.width*s/2)) && mouseX < (x + (imageSliderKnob.width*s/2)) && mouseY > (y - (imageSliderKnob.height*s/2)) && mouseY < (y + (imageSliderKnob.height*s/2))) {
      return true;
    }
    return false;
  }
}

class Kompass {
  PImage imageAuto;
  PImage imageArm;
  float x;   //Cebter x pos
  float y;   //Center y pos
  float r;   //radius  
  float d;   //distance Auto Arm (Center - Center)
  float yoff;//offset Arm Auto 
  float s;   //scale factor

  Kompass(PImage imageCar, PImage imageA, float xpos, float ypos, float radius, float distance, float yOffset, float scaleK) {
    imageAuto = imageCar;
    imageArm = imageA;
    x = xpos;
    y = ypos;
    r = radius;
    d = distance;
    yoff = yOffset;
    s = scaleK;
  }

  void update() {
    pushMatrix();
    translate(x, y);
    fill(188, 170, 148);
    strokeWeight(15);
    ellipseMode(CENTER);
    stroke(77, 77, 77);
    ellipse(0, 0, r+30, r+30);
    stroke(97, 95, 95);
    ellipse(0, 0, r, r);
    noStroke();
    rotate(radians(rotation));
    image(imageAuto, 0, 0, (int)s*imageAuto.width, (int)s*imageAuto.height);
    if (armDown)
      image(imageArm, d, yoff, (int)s*imageArm.width, (int)s*imageArm.height);
    popMatrix();
  }
}
