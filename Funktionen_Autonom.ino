
void sendDebug() {
  HC06.print("VL: ");
  HC06.print(black(4));
  HC06.print(" VR: ");
  HC06.print(black(5));
  HC06.print(" M1: ");
  HC06.print(black(1));
  HC06.print(" M2: ");
  HC06.print(black(2));
  HC06.print(" M3: ");
  HC06.print(black(3));
  HC06.print(" U1: ");
  HC06.print(disFront);
  HC06.print("cm U2: ");
  HC06.print(disBack);
  HC06.println("cm");
}

void followTheLine() {
  straight = true;
  Serial.print("MR: ");
  Serial.print(black(1));
  Serial.print(" - ");
  Serial.print(analogRead(SWM1Pin));
  Serial.print("  MM: ");
  Serial.print(black(2));
  Serial.print(" - ");
  Serial.print(analogRead(SWM2Pin));
  Serial.print("  ML: ");
  Serial.print(black(3));
  Serial.print(" - ");
  Serial.print(analogRead(SWM3Pin));
  Serial.print("  VL: ");
  Serial.print(black(4));
  Serial.print(" - ");
  Serial.print(analogRead(SWVLPin));
  Serial.print("  VR: ");
  Serial.print(black(5));
  Serial.print(" - ");
  Serial.print(analogRead(SWVRPin));
  Serial.print("  thisWinkel: ");
  Serial.print(thisWinkel);
  Serial.print("  Winkel: ");
  Serial.print(winkel);
  Serial.print("  deltaWinkel: ");
  Serial.print(deltaWinkel(winkel, thisWinkel));
  Serial.print("  curveTime: ");
  Serial.println(curveTime);

  if ((black(4) || black(5)) && !curveNext && curveTime > 50) {
    Serial.println("Kreuzung!");
    inCrossing = true;
    followLine = false;
    whiteTime = 0;
  } else if (!black(1) && !black(2) && !black(3) &&  curveNext && disFront < 13) { //linkskurve 90
    thisWinkel = toDegree(thisWinkel - 89);
    Serial.println("Linkskurve!");
    curveNext = false;
    whiteTime = 0;
    curveTime = 0;
  } else if ((black(4) || black(5)) && curveNext && disFront < 21) {   //rechtskurve
    thisWinkel = toDegree(thisWinkel + 90);
    Serial.println("Rechtskurve!");
    curveNext = false;
    whiteTime = 0;
    curveTime = 0;
  } else if (deltaWinkel(winkel, thisWinkel) > 45) {    //drehung ausgleichen extrem
    makeCurve(0, 0, true, 10);
    Serial.println("Kurve1extrem");
    whiteTime = 0;
  } else if (deltaWinkel(winkel, thisWinkel) < -45) {
    makeCurve(0, 0, false, 10);
    Serial.println("Kurve2extrem");
    whiteTime = 0;
  } else if (deltaWinkel(winkel, thisWinkel) > 3) {    //drehung ausgleichen    //davor > 2
    makeCurve(0, 60, true, 10);   //davor 70  //davor 55
    Serial.println("Kurve1");
    whiteTime = 0;
  } else if (deltaWinkel(winkel, thisWinkel) < -3) {   //davor > 2
    makeCurve(0, 50, false, 10);  // davor 50   //davor 45
    Serial.println("Kurve2");
    whiteTime = 0;
  } else if (black(2)) { //gut -> gerade
    moveStraight(thisWinkel, true, 10);
    Serial.println("Gerade!");
    whiteTime = 0;
  } else if (black(1)) { //zu weit rechts
    moveToSide(true, 10);
    Serial.println("Links");
    whiteTime = 0;
  } else if (black(3)) { //zu weit links
    moveToSide(false, 10);
    Serial.println("Rechts!");
    whiteTime = 0;
  } else if (whiteTime < 10) { //alle SW Sensoren weiß aber noch nicht lange
    detachServos(false);
    whiteTime++;
    Serial.println("detach");
  } else {
    followLine = false;
    searchingLine = true;
    whiteTime = 0;
  }
  curveTime++;
}

void cross(int crossdirection) {  //0 = gerade, 1 = links, 2 = rechts     //muss noch kalibriert werden
  straight = true;
  int cd = CROSS_DURATION;
  int i = 1;
  bool kurveFertig;
  if (crossdirection == 2) i = -1;
  if (crossdirection != 0) cd += 100 - ((crossdirection - 1) * 50);
  if (crossdirection == 0 || (crossTime < 100 && crossdirection == 1) || (crossTime < 60 && crossdirection == 2)) {
    moveStraight(thisWinkel, true, 10);
    kurveFertig = false;
  } else if (abs(deltaWinkel(winkel, toDegree(thisWinkel - (90*i)))) > 5 && !kurveFertig) {
    if (crossdirection == 1) {
      makeCurve(0, 0, false, 10);
    } else if (crossdirection == 2) {
      makeCurve(0, 0, true, 10);
    }
  } else {
    kurveFertig = true;
    if (crossdirection == 1) {
      moveStraight(toDegree(thisWinkel - 90), true, 10);
    } else if (crossdirection == 2) {
      moveStraight(toDegree(thisWinkel + 90), true, 10);
    }
  }
  if ((black(1) || black(2) || black(3)) && crossTime > 100) {
    if (crossdirection == 1) thisWinkel = toDegree(thisWinkel - 90);   
    if (crossdirection == 2) thisWinkel = toDegree(thisWinkel + 90);
    crossTime = 0;
    followLine = true;
    crossCounter++;
    // curveNext = true;
    inCrossing = false;
  }
  if (crossTime >= cd) {
    if (crossdirection == 1) thisWinkel = toDegree(thisWinkel - 90);    
    if (crossdirection == 2) thisWinkel = toDegree(thisWinkel + 90);
    crossTime = 0;
    searchingLine = true;
    inCrossing = false;
  }
  Serial.print("CrossTime: ");
  Serial.print(crossTime);
  Serial.print("  crossdirection: ");
  Serial.println(crossdirection);
  crossTime++;
}

void searchLine() {   //after crossing or curve if middle line was not be found
  Serial.println("searchLine!");
  if (searchTime > (20 + (searchCount * 10))) {
    searchTime = -20 - (searchCount * 10);
    searchCount++;
  }
  if (searchTime < 0)
    moveToSide(true, 20);
  if (searchTime >= 0)
    moveToSide(false, 20);
  if (black(1) || black(2) || black(3)) {
    searchTime = 10;
    crossTime = 0;
    searchCount = 0;
    followLine = true;
    searchingLine = false;
    Serial.println("Linie gefunden! :)");
  }
  //if (crossTime > SEARCH_DURATION) {
  if (searchCount > 3) {
    searchTime = 10;
    crossTime = 0;
    searchCount = 0;
    detachServos(false);
    searchingLine = false;
    Serial.println("keine Linie gefunden! :(");
  }
  searchTime ++;
  crossTime ++;
}

