


void loop(void)
{
  processAccelGyro();
  if (!calibratedSW)
    calibrateSW();
  if ((millis() - timeMil) > SEND_DELAY) {
    strOut = (int)winkel;
    strOut += "|";
    strOut += statusCar;
    strOut += "|";
    strOut += disFront;
    strOut += "|";
    strOut += disBack;
    strOut += "E";
    HC06.print(strOut);
    //Serial.println(strOut);
    disFront = checkDistance(true);
    mpu.resetFIFO();
    disBack = checkDistance(false);
    strOut = "";
    timeMil = millis();
  }

  if (isCalibrate) {

    while (HC06.available()) {
      chByte = HC06.read();
      if (chByte == 'E') {
        bleDat[counter] = strToInt(strInput);
        counter = 0;
        bleSpeed =  bleDat[3] - 100;
        bleCurve =  bleDat[4] - 100;
        strInput = "";
      } else if (chByte == '|') {
        bleDat[counter] = strToInt(strInput);
        counter++;
        strInput = "";
      } else {
        strInput += chByte;
      }
    }

    if (bleDat[2] == 0 && bleSpeed != 0 && bleCurve == 0) {
      straight = true;
    } else {
      straight = false;
    }

    if (bleDat[0] != 0 && bleDat[1] != 0) { //Fahre autonom
      followLine = true;  //nur nach Line ohne calcWay
      //calcWay();
      thisWinkel = winkel;
      straight = true;
      oldStraight = true;
      statusCar++;
      bleDat[0] = 0;
      bleDat[1] = 0;
      //Serial.println("Autonom!");
      //...
    } else if (bleDat[2] != 0) { //Fahre nach Buttons
      switch (bleDat[2]) {
        case 1: moveToSide(true, 100);  break;        //links
        case 2: moveToSide(false, 100); break;       //rechts
        case 3: makeCurve(0, 0, true, 100); /*if (calibratedSW) {   //aktivieren um autonomen Modus manuell starten und abbrechen zu können 
            followLine = false;
            crossCounter = 0;
            curveNext = false;
          } else {
            SWinCal = true;
          }*/ break; //auf der Stelle drehen links
        case 4: makeCurve(0, 0, false, 100); /*followLine = true; thisWinkel = winkel;      
            straight = true;
            oldStraight = true;*/ break;  //auf der Stelle drehen rechts
        case 5: moveArm(false); break;          //Arm runter
        case 6: moveArm(true); break;           //Arm hoch
        default: Serial.println("Kommunikationsfehler!"); break;
      }
    } else if (bleSpeed > 0 && bleCurve != 0) { //Fahre Kurve
      makeCurve(0, map(abs(bleCurve), 1, 100, 100, 1), bleCurve > 0, abs(bleSpeed));
    } else if ((bleSpeed != 0 && bleCurve == 0) || (bleSpeed < 0 && bleCurve != 0)) { //Fahre geradeaus
      moveStraight(thisWinkel, bleSpeed > 0, abs(bleSpeed));
    } else {   //Stoppe
      if (!followLine && !inCrossing && !searchingLine) {
        detachServos(false);
      }
    }
    if (SWinCal) {
      calibrateSW();
    }
    if (followLine) {
      followTheLine();
    }
    if (inCrossing) {
      cross(crossDirs[crossCounter]);
    }
    if (searchingLine) {
      searchLine();
    }
    if (waiting) {
      wait();
    }
    if (ending) {
      theEnd();
    }
    if (straight != oldStraight) {
      thisWinkel = winkel;
      oldStraight = straight;
    }

  }
}
