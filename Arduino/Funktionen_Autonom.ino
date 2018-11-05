

void followTheLine() {
  straight = true;
  //nur fürs Testen:
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

  if (disFront < 5) { //hinderniss -> Stop!
    Serial.println("Stop!");
    detachServos(false);
    whiteTime = 0;
  } else if (crossDirs[crossCounter] == 4) { //Stop an Start Ziel    //erstmal überspringen
    crossCounter++;
    statusCar++;
    //statusCar++; // -> nur gebraucht ohne waiting
    timeCount = millis();
    waiting = true;
  } else if (crossDirs[crossCounter] == 5) { //Wieder an Ladestation -> followLine = false
    followLine = false;
    ending = true;
  } else if ((black(4) || black(5)) && crossDirs[crossCounter] <= 3 && crossDirs[crossCounter] != 0 && curveTime > 50) {   //Kreuzung
    Serial.print(crossDirs[crossCounter]);
    Serial.println("  Kreuzung!");
    inCrossing = true;
    followLine = false;
    whiteTime = 0;
  } else if (!black(1) && !black(2) && !black(3) &&  crossDirs[crossCounter] == 6 && disFront < 13) { //linkskurve 90
    thisWinkel = toDegree(thisWinkel - 90);   //vorher -89
    Serial.print(crossDirs[crossCounter]);
    Serial.println("  Linkskurve!");
    crossCounter++;
    curveNext = false;
    whiteTime = 0;
    curveTime = 0;
  } else if ((black(4) || black(5)) && crossDirs[crossCounter] == 6 && disFront < 21) {   //rechtskurve
    thisWinkel = toDegree(thisWinkel + 90);
    Serial.print(crossDirs[crossCounter]);
    Serial.println("  Rechtskurve!");
    crossCounter++;
    curveNext = false;
    whiteTime = 0;
    curveTime = 0;
  } else if (deltaWinkel(winkel, thisWinkel) > 45) {    //drehung ausgleichen extrem
    makeCurve(0, 0, true, 10);
    Serial.print(crossDirs[crossCounter]);
    Serial.println("  Kurve1extrem");
    whiteTime = 0;
  } else if (deltaWinkel(winkel, thisWinkel) < -45) {
    makeCurve(0, 0, false, 10);
    Serial.print(crossDirs[crossCounter]);
    Serial.println("  Kurve2extrem");
    whiteTime = 0;
  } else if (deltaWinkel(winkel, thisWinkel) > 3) {    //drehung ausgleichen    //davor > 2
    makeCurve(0, 60, true, 10);   //davor 70  //davor 55
    Serial.print(crossDirs[crossCounter]);
    Serial.println("  Kurve1");
    whiteTime = 0;
  } else if (deltaWinkel(winkel, thisWinkel) < -3) {   //davor > 2
    makeCurve(0, 50, false, 10);  // davor 50   //davor 45
    Serial.print(crossDirs[crossCounter]);
    Serial.println("  Kurve2");
    whiteTime = 0;
  } else if (black(2)) { //gut -> gerade
    moveStraight(thisWinkel, true, 10);
    Serial.print(crossDirs[crossCounter]);
    Serial.println("  Gerade!");
    whiteTime = 0;
  } else if (black(1)) { //zu weit rechts
    moveToSide(true, 10);
    Serial.print(crossDirs[crossCounter]);
    Serial.println("  Links");
    whiteTime = 0;
  } else if (black(3)) { //zu weit links
    moveToSide(false, 10);
    Serial.print(crossDirs[crossCounter]);
    Serial.println("  Rechts!");
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

void cross(int crossdirection) {  //0 = gerade, 1 = links, 2 = rechts, 3 = auch rechts
  straight = true;
  if (disFront < 7) {       //Hinderniss -> Stop
    detachServos(false);
  } else {
    int cd = CROSS_DURATION - 20; //normal ohne - 20
    int i = 1;
    bool kurveFertig;
    if (crossdirection == 3) crossdirection = 0;
    if (crossdirection == 2) i = -1;
    if (crossdirection != 0) cd += 100 - ((crossdirection - 1) * 50);
    if (crossdirection == 0 || (crossTime < 80 && crossdirection == 1) || (crossTime < 60 && crossdirection == 2)) {
      moveStraight(thisWinkel, true, 10);
      kurveFertig = false;
    } else if (abs(deltaWinkel(winkel, toDegree(thisWinkel - (90 * i)))) > 5 && !kurveFertig) {
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
      curveTime = 40;   //vorher 25
      // curveNext = true;
      inCrossing = false;
    }
    if (crossTime >= cd) {
      if (crossdirection == 1) thisWinkel = toDegree(thisWinkel - 90);
      if (crossdirection == 2) thisWinkel = toDegree(thisWinkel + 90);
      crossTime = 0;
      crossCounter++;
      curveTime = 40;   //vorher 25
      searchingLine = true;
      inCrossing = false;
    }
    Serial.print("CrossTime: ");
    Serial.print(crossTime);
    Serial.print("  crossdirection: ");
    Serial.println(crossdirection);

    crossTime++;
  }
}

void searchLine() {   //after crossing or curve if middle line was not be found
  straight = true;
  Serial.println("searchLine!");
  if (searchTime > (20 + (searchCount * 18))) {
    searchCount++;
    searchTime = -20 - (searchCount * 10);
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

void calcWay() {  //1 = blau(VL), 2 = gelb(VR), 3 = grün(HL), 4 = rot(HR)
  //crossDirs[] -> 1 = links, 2 = rechts, 3 = gerade, 4 = stop, 5 = Ende, 6 = Kurve
  for (int i = 0; i < (sizeof(crossDirs) / sizeof(uint8_t)); i++) {
    crossDirs[i] = 0;
  }
  if (bleDat[0] == 1) { // Start = blau
    crossDirs[0] = 1; crossDirs[1] = 6; crossDirs[2] = 3; crossDirs[3] = 4;
    if (bleDat[1] == 2) { //Ziel = gelb
      crossDirs[4] = 6; crossDirs[5] = 3; crossDirs[6] = 6; crossDirs[7] = 4; crossDirs[8] = 3; crossDirs[9] = 6; crossDirs[10] = 1; crossDirs[11] = 5;
    } else if (bleDat[1] == 3) { //Ziel = grün
      crossDirs[4] = 6; crossDirs[5] = 2; crossDirs[6] = 2; crossDirs[7] = 1; crossDirs[8] = 4; crossDirs[9] = 6; crossDirs[10] = 2; crossDirs[11] = 5;
    } else if (bleDat[1] == 4) { //Ziel = rot
      crossDirs[4] = 6; crossDirs[5] = 2; crossDirs[6] = 1; crossDirs[7] = 2; crossDirs[8] = 4; crossDirs[9] = 6; crossDirs[10] = 1; crossDirs[11] = 5;
    }
  } else if (bleDat[0] == 2) { // Start = gelb
    crossDirs[0] = 2; crossDirs[1] = 6; crossDirs[2] = 3; crossDirs[3] = 4;
    if (bleDat[1] == 1) { //Ziel = blau
      crossDirs[4] = 6; crossDirs[5] = 3; crossDirs[6] = 6; crossDirs[7] = 4; crossDirs[8] = 3; crossDirs[9] = 6; crossDirs[10] = 2; crossDirs[11] = 5;
    } else if (bleDat[1] == 3) { //Ziel = grün
      crossDirs[4] = 6; crossDirs[5] = 1; crossDirs[6] = 2; crossDirs[7] = 1; crossDirs[8] = 4; crossDirs[9] = 6; crossDirs[10] = 2; crossDirs[11] = 5;
    } else if (bleDat[1] == 4) { //Ziel = rot
      crossDirs[4] = 6; crossDirs[5] = 1; crossDirs[6] = 1; crossDirs[7] = 2; crossDirs[8] = 4; crossDirs[9] = 6; crossDirs[10] = 1; crossDirs[11] = 5;
    }
  } else if (bleDat[0] == 3) { // Start = grün
    crossDirs[0] = 1; crossDirs[1] = 6; crossDirs[2] = 4;
    if (bleDat[1] == 1) { //Ziel = blau
      crossDirs[3] = 3; crossDirs[4] = 4; crossDirs[5] = 6; crossDirs[6] = 2; crossDirs[7] = 3; crossDirs[8] = 3; crossDirs[9] = 5;
    } else if (bleDat[1] == 2) { //Ziel = gelb
      crossDirs[3] = 2; crossDirs[4] = 3; crossDirs[5] = 1; crossDirs[6] = 4; crossDirs[7] = 6; crossDirs[8] = 1; crossDirs[9] = 3; crossDirs[10] = 3; crossDirs[11] = 5;
    } else if (bleDat[1] == 4) { //Ziel = rot
      crossDirs[3] = 2; crossDirs[4] = 3; crossDirs[5] = 2; crossDirs[6] = 4; crossDirs[7] = 6; crossDirs[8] = 1; crossDirs[9] = 5;
    }
  } else if (bleDat[0] == 4) { // Start = rot
    crossDirs[0] = 2; crossDirs[1] = 6; crossDirs[2] = 4;
    if (bleDat[1] == 1) { //Ziel = blau
      crossDirs[3] = 1; crossDirs[4] = 3; crossDirs[5] = 2; crossDirs[6] = 4; crossDirs[7] = 6; crossDirs[8] = 2; crossDirs[9] = 3; crossDirs[10] = 3; crossDirs[11] = 5;
    } else if (bleDat[1] == 2) { //Ziel = gelb
      crossDirs[3] = 3; crossDirs[4] = 4; crossDirs[5] = 6; crossDirs[6] = 1; crossDirs[7] = 3; crossDirs[8] = 3; crossDirs[9] = 5;
    } else if (bleDat[1] == 3) { //Ziel = grün
      crossDirs[3] = 1; crossDirs[4] = 3; crossDirs[5] = 1; crossDirs[6] = 4; crossDirs[7] = 6; crossDirs[8] = 2; crossDirs[9] = 5;
    }
  } else {
    Serial.println("falsche Zieleingabe!");
  }
  Serial.print("crossDirs: ");
  Serial.print(crossDirs[0]);
  Serial.print(" , ");
  Serial.print(crossDirs[1]);
  Serial.print(" , ");
  Serial.print(crossDirs[2]);
  Serial.print(" , ");
  Serial.print(crossDirs[3]);
  Serial.print(" , ");
  Serial.print(crossDirs[4]);
  Serial.print(" , ");
  Serial.print(crossDirs[5]);
  Serial.print(" , ");
  Serial.print(crossDirs[6]);
  Serial.print(" , ");
  Serial.print(crossDirs[7]);
  Serial.print(" , ");
  Serial.print(crossDirs[8]);
  Serial.print(" , ");
  Serial.print(crossDirs[9]);
  Serial.print(" , ");
  Serial.print(crossDirs[10]);
  Serial.print(" , ");
  Serial.println(crossDirs[11]);
  followLine = true;
}

void wait() {     //neu -> ???
  straight = true;
  int w = 7000;
  if (crossDirs[crossCounter - 2] == 3)
    w  -= 4000;
  if ((millis() - timeCount) > (w + 5000)) { //5 sek
    waiting = false;
    curveTime = 51;
    statusCar++;
    whiteTime = 0;
    followLine = true;
    Serial.print(millis() - timeCount);
    Serial.println("  weiter");
    //crossTime = 0;
  } else if ((millis() - timeCount) > w) { //7 sek
    followLine = false;
    detachServos(false);
    Serial.print(millis() - timeCount);
    Serial.println("  wait");
  } else {
    Serial.print(millis() - timeCount);
    Serial.println("  warte");
  }
}

void theEnd() {
  straight = true;
  moveStraight(thisWinkel, true, 10);
  if ((millis() - timeCount) > 4000) { //4 sek
    detachServos(false);
    ending = false;
    Serial.println("Wieder am Parkplatz!");
  }
}

