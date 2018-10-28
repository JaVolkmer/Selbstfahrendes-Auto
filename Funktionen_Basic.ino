int strToInt(String str) {
  char this_char[str.length() + 1];
  str.toCharArray(this_char, sizeof(this_char));
  int intval = atoi(this_char);
  return intval;
}

void processAccelGyro()
{
  winkelOld = winkel;
  calNumberOld = calNumber;
  mpuIntStatus = mpu.getIntStatus();
  fifoCount = mpu.getFIFOCount();
  if ((mpuIntStatus & 0x10) || fifoCount == 1024)
  {
    mpu.resetFIFO();
    Serial.println(F("FIFO overflow!"));
    return;
  }
  if (mpuIntStatus & 0x02)  // otherwise continue processing
  {
    if (fifoCount < packetSize)
      return; //  fifoCount = mpu.getFIFOCount();
    mpu.getFIFOBytes(fifoBuffer, packetSize);
    fifoCount -= packetSize;
    mpu.resetFIFO();
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    mpuPitch = ypr[PITCH] * 180 / M_PI;
    mpuRoll = ypr[ROLL] * 180 / M_PI;
    mpuYaw  = ypr[YAW] * 180 / M_PI;

    mpu.resetFIFO();

    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);

    mpu.resetFIFO();
    float winkelGenau;
    if (!isCalibrate) {
      calNumber = mpuYaw;
      if (abs(calNumber - calNumberOld) == 0.00 && millis() > 1000) {
        calValue = calNumber;
        Serial.println("Fertig Kalibriert!");
        isCalibrate = true;
        statusCar = 1;
      }
    } else {
      if (mpuYaw - calValue >= 0) {
        winkelGenau = (int)(((mpuYaw - calValue) * 100 + 0.5) / 10.0);
      } else {
        winkelGenau = (int)((((mpuYaw - calValue) + 360) * 100 + 0.5) / 10.0);
      }
      winkel = winkelGenau / 10;
      if (winkel == 360) winkel = 0;
      if (winkel + 20 < winkelOld || winkel - 20 > winkelOld) {
        if (winkelOld < 20 && winkel > 340 || winkelOld > 340 && winkel < 20) {} else {
          winkel = winkelOld;
          Serial.println("Fehler korrigiert!");
        }
      }
    }
    mpu.resetFIFO();
  }
}

void detachServos(bool all) { //true = all Servos (+Arm)
  servo1.detach();
  servo2.detach();
  servo3.detach();
  servo4.detach();
  if (all) servoArm.detach();
}

void attachServos(bool all) { //true = all Servos (+Arm)
  servo1.attach(ServoPin1);
  servo2.attach(ServoPin2);
  servo3.attach(ServoPin3);
  servo4.attach(ServoPin4);
  if (all) servoArm.attach(ServoArmPin);
}

void driveServo(int servoNum, int servoSpeed) {
  if (servoSpeed > 100 || servoSpeed < -100) return;
  if (servoNum == 1) {
    if (servoSpeed == 0) {
      servo1.detach();
    } else {
      servo1.attach(ServoPin1);
    }
    if (servoSpeed > 0) servo1.write(map(servoSpeed, 0, 100, ZERO_SPEED, 180));
    if (servoSpeed < 0) servo1.write(map(servoSpeed, 0, -100, ZERO_SPEED, 0));
  } else if (servoNum == 2) {
    if (servoSpeed == 0) {
      servo2.detach();
    } else {
      servo2.attach(ServoPin2);
    }
    if (servoSpeed > 0) servo2.write(map(servoSpeed, 0, 100, ZERO_SPEED, 0));
    if (servoSpeed < 0) servo2.write(map(servoSpeed, 0, -100, ZERO_SPEED, 180));
  } else if (servoNum == 3) {
    if (servoSpeed == 0) {
      servo3.detach();
    } else {
      servo3.attach(ServoPin3);
    }
    if (servoSpeed > 0) servo3.write(map(servoSpeed, 0, 100, ZERO_SPEED, 180));
    if (servoSpeed < 0) servo3.write(map(servoSpeed, 0, -100, ZERO_SPEED, 0));
  } else if (servoNum == 4) {
    if (servoSpeed == 0) {
      servo4.detach();
    } else {
      servo4.attach(ServoPin4);
    }
    if (servoSpeed > 0) servo4.write(map(servoSpeed, 0, 100, ZERO_SPEED, 0));
    if (servoSpeed < 0) servo4.write(map(servoSpeed, 0, -100, ZERO_SPEED, 180));
  } else {
    return;
  }
}

int toDegree (int degree) {
  if (degree >= 360) return (degree - 360);
  if (degree < 0) return (360 + degree);
  return (degree);
}

void moveStraight(float sollWinkel, bool driveForeward, int sSpeed) {
  if (sSpeed > 100 || sSpeed < 0) return;
  int multiplier = 1;
  if (!driveForeward) multiplier = -1;
  if (winkel > toDegree(sollWinkel + ABWEICHUNG_MIN) && winkel < toDegree(sollWinkel + ABWEICHUNG_MAX) && driveForeward || winkel < toDegree(sollWinkel - ABWEICHUNG_MIN) && winkel > toDegree(sollWinkel - ABWEICHUNG_MAX) && !driveForeward) { // wenn rechts vorne / links hinten -> nach links
    driveServo(1, ((sSpeed / 100)*CORRECT_SPEED_LEFT) * multiplier);
    driveServo(2, sSpeed * multiplier);
    driveServo(3, ((sSpeed / 100)*CORRECT_SPEED_LEFT) * multiplier);
    driveServo(4, sSpeed * multiplier);
    //Serial.println("Links!");
  } else if (winkel < toDegree(sollWinkel - ABWEICHUNG_MIN) && winkel > toDegree(sollWinkel - ABWEICHUNG_MAX) && driveForeward || winkel > toDegree(sollWinkel + ABWEICHUNG_MIN) && winkel < toDegree(sollWinkel + ABWEICHUNG_MAX) && !driveForeward) {  // wenn links vorne / rechts hinten -> nach rechts
    driveServo(1, sSpeed * multiplier);
    driveServo(2, ((sSpeed / 100)*CORRECT_SPEED_RIGHT) * multiplier);
    driveServo(3, sSpeed * multiplier);
    driveServo(4, ((sSpeed / 100)*CORRECT_SPEED_RIGHT) * multiplier);
    //Serial.println("Rechts!");
  } else if (winkel > toDegree(sollWinkel + ABWEICHUNG_MAX) && winkel < toDegree(sollWinkel - ABWEICHUNG_MAX)) { // stop
    driveServo(1, 0);
    driveServo(2, 0);
    driveServo(3, 0);
    driveServo(4, 0);
    Serial.println("Stop!");
  } else {  // geradeaus
    driveServo(1, sSpeed * multiplier);
    driveServo(2, sSpeed * multiplier);
    driveServo(3, sSpeed * multiplier);
    driveServo(4, sSpeed * multiplier);
    //Serial.println("Gerade aus!");
  }
}

void makeCurve(int degree, int radius, bool curveDirection, int cSpeed) { //degree: +-180 von winkel, radius 1-100 / wenn radius = 0 -> auf der Stelle drehen, curveDirection true = links
  if (degree >= 360 || degree < 0) return;
  if (radius > 100 || radius < 0) return;
  if (cSpeed > 100 || cSpeed < 0) return;
  if (degree < toDegree(winkel + ABWEICHUNG_MIN) && degree > toDegree(winkel - ABWEICHUNG_MIN)) {
    detachServos(false);
    return;
  }
  int multiplier = 1;
  if (curveDirection) multiplier = -1;
  if (radius == 0) {
    driveServo(1, -cSpeed * multiplier);
    driveServo(2, cSpeed * multiplier);
    driveServo(3, -cSpeed * multiplier);
    driveServo(4, cSpeed * multiplier);
  } else {
    int curveSpeed = map(radius, 1, 100, 0, 100) * ((float)cSpeed / 100);
    //Serial.print("Kurve! ");
    //Serial.println(curveSpeed);
    if (!curveDirection) {
      driveServo(1, curveSpeed);
      driveServo(2, cSpeed);
      driveServo(3, curveSpeed);
      driveServo(4, cSpeed);
    } else {
      driveServo(1, cSpeed);
      driveServo(2, curveSpeed);
      driveServo(3, cSpeed);
      driveServo(4, curveSpeed);
    }
  }
}

void moveToSide(bool side, int sideSpeed) { //true = rechts
  int multiplier = 1;
  if (side) multiplier = -1;
  driveServo(1, -sideSpeed * multiplier);
  driveServo(2, sideSpeed * multiplier);
  driveServo(3, sideSpeed * multiplier);
  driveServo(4, -sideSpeed * multiplier);
}

void moveArm(bool up) {
  servoArm.attach(ServoArmPin);
  if (up) {
    servoArm.write(ARM_UP_POS);
    armUp = true;
  }
  else {
    servoArm.write(ARM_DOWN_POS);
    armUp = false;
  }
}

bool black(int sensorNum) { //Sensornum: 1->MR, 2->MM, 3->ML, 4->VL, 5->VR
  if (calibratedSW) {
    //int value;
    bool b = false;
    switch (sensorNum) {
      case 1: if (analogRead(SWM1Pin) > SWMR_Cal)b = true; break;
      case 2: if (analogRead(SWM2Pin) > SWMM_Cal)b = true; break;
      case 3: if (analogRead(SWM3Pin) > SWML_Cal)b = true; break;
      case 4: if (analogRead(SWVLPin) > SWVL_Cal)b = true; break;
      case 5: if (analogRead(SWVRPin) > SWVR_Cal)b = true; break;
      default: return (false); break;
    }
    /*
      switch (sensorNum) {
      case 1: value = analogRead(SWM1Pin)-SW_Sensor_M1; break;
      case 2: value = analogRead(SWM2Pin)-SW_Sensor_M2; break;
      case 3: value = analogRead(SWM3Pin)-SW_Sensor_M3; break;
      case 4: value = analogRead(SWVLPin)-SW_Sensor_VL; break;
      case 5: value = analogRead(SWVRPin)-SW_Sensor_VR; break;
      default: return (false); break;
      }

      if (value > SW_Sensor_VALUE) {
      return (true);
      } else {
      return (false);
      }
    */
    if (b) {
      return (true);
    } else {
      return (false);
    }
  } else {
    return (false);
  }
}

int checkDistance(bool front) {
  int entfernung;
  if (front) {
    digitalWrite(Ultra1TriggerPin, LOW);
    delay(5);
    digitalWrite(Ultra1TriggerPin, HIGH);
    delay(10);
    digitalWrite(Ultra1TriggerPin, LOW);
    entfernung = (pulseIn(Ultra1EchoPin, HIGH, 20000) / 2) * ULTRA_CALIBRATE;
  } else {
    digitalWrite(Ultra2TriggerPin, LOW);
    delay(5);
    digitalWrite(Ultra2TriggerPin, HIGH);
    delay(10);
    digitalWrite(Ultra2TriggerPin, LOW);
    entfernung = (pulseIn(Ultra2EchoPin, HIGH, 20000) / 2) * ULTRA_CALIBRATE;
  }
  if (entfernung >= 200 || entfernung <= 0) {
    return (0);
  } else {
    return (entfernung);
  }
}

int deltaWinkel(int winkel1, int winkel2) { //wie weit ist winkel 1 von winkel 2 entfernt? (w2-w1) -> output +-180 -> + = w2 weiter rechts als w1
  int offset = 180 - toDegree(winkel1);
  winkel2 = toDegree(offset + toDegree(winkel2));
  return winkel2 - 180;
}

void calibrateSW() {
  /*
    if (calibrationCounter == 0) {
    SWML_Cal = analogRead(SWM3Pin) + SW_PUFFER;
    SWMM_Cal = analogRead(SWM2Pin) + SW_PUFFER;
    SWMR_Cal = analogRead(SWM1Pin) + SW_PUFFER;
    SWVL_Cal = analogRead(SWVLPin) + SW_PUFFER;
    SWVR_Cal = analogRead(SWVRPin) + SW_PUFFER;
    }else{
  */
  SWML_Cal += analogRead(SWM3Pin);
  SWMM_Cal += analogRead(SWM2Pin);
  SWMR_Cal += analogRead(SWM1Pin);
  SWVL_Cal += analogRead(SWVLPin);
  SWVR_Cal += analogRead(SWVRPin);
  Serial.println(calibrationCounter);
  //}
  if (calibrationCounter >= 29) { //30 durchgänge
    SWML_Cal = (SWML_Cal / 30) + SW_PUFFER;
    SWMM_Cal = (SWMM_Cal / 30) + SW_PUFFER;
    SWMR_Cal = (SWMR_Cal / 30) + SW_PUFFER;
    SWVL_Cal = (SWVL_Cal / 30) + SW_PUFFER;
    SWVR_Cal = (SWVR_Cal / 30) + SW_PUFFER;
    Serial.print("SW Kalibriert! - ML: ");
    Serial.print(SWML_Cal);
    Serial.print("  MM: ");
    Serial.print(SWMM_Cal);
    Serial.print("  MR: ");
    Serial.print(SWMR_Cal);
    Serial.print("  VL: ");
    Serial.print(SWVL_Cal);
    Serial.print("  VR: ");
    Serial.print(SWVR_Cal);
    calibratedSW = true;
    SWinCal = false;
    calibrationCounter = 0;
  }
  calibrationCounter++;
}

