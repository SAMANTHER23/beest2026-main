#include "vex.h"

Drive::Drive(motor_group leftDrive, motor_group rightDrive, inertial inertialSensor, float wheelDiameter, float gearRatio):
  leftDrive(leftDrive),
  rightDrive(rightDrive),
  wheelDiameter(wheelDiameter),
  gearRatio(gearRatio),
  inertialSensor(inertialSensor) {}

void Drive::setMaxVoltage(float turnMaxVoltage, float driveMaxVoltage, float headingMaxVoltage)
{ 
  this -> driveMaxVoltage = driveMaxVoltage;
  this -> turnMaxVoltage = turnMaxVoltage;
  this -> headingMaxVoltage = headingMaxVoltage;
}

void Drive::setTurnPID(float turnKp, float turnKi, float turnKd, float turnStarti) {
  this -> turnKp = turnKp;
  this -> turnKi = turnKi;
  this -> turnKd = turnKd;
  this -> turnStarti = turnStarti;
}

void Drive::setDrivePID(float driveKp, float driveKi, float driveKd, float driveStarti) {
  this -> driveKp = driveKp;
  this -> driveKi = driveKi;
  this -> driveKd = driveKd;
  this -> driveStarti = driveStarti;
}

void Drive::setHeadingPID(float headingKp, float headingKd) {
  this -> headingKp = headingKp;
  this -> headingKd = headingKd;
}

void Drive::setTurnExitConditions(float turnSettleError, float turnSettleTime, float turnTimeout) {
  this -> turnSettleError = turnSettleError;
  this -> turnSettleTime = turnSettleTime;
  this -> turnTimeout = turnTimeout;
}

void Drive::setDriveExitConditions(float driveSettleError, float driveSettleTime, float driveTimeout) {
  this -> driveSettleError = driveSettleError;
  this -> driveSettleTime = driveSettleTime;
  this -> driveTimeout = driveTimeout;
}

void Drive::setGyroScale(float gyroScale)
{
  gyro_scale = gyroScale;
}

void Drive::setHeading(float orientationDeg) {
  inertialSensor.setHeading(orientationDeg*gyro_scale/360.0, deg);
  targetHeading = orientationDeg*gyro_scale/360.0;
}

float Drive::getHeading() {
  return( normalize360( inertialSensor.heading()*360.0/gyro_scale ) ); 
}

float Drive::getLeftPosition() {
  return leftDrive.position(deg) / 360.0 * gearRatio  * M_PI * wheelDiameter;
}

float Drive::getRightPosition() {
  return rightDrive.position(deg) / 360.0 * gearRatio * M_PI * wheelDiameter;
}

void Drive::driveWithVoltage(float leftVoltage, float rightVoltage) {
  leftDrive.spin(fwd, leftVoltage, volt);
  rightDrive.spin(fwd, rightVoltage, volt);
}

void Drive::turnToHeading(float heading) {
  turnToHeading(heading, turnMaxVoltage);
}


void Drive::turnToHeading(float heading, float turnMaxVoltage, float earlyExitFactor) {
  if (earlyExitFactor > 10) earlyExitFactor = 10;
  if (earlyExitFactor < 1) earlyExitFactor = 1;
  targetHeading = normalize360(heading);
  PID turnPID(turnKp, turnKi, turnKd, turnStarti, turnSettleError*earlyExitFactor , turnSettleTime/earlyExitFactor, turnTimeout);
  while (!turnPID.isDone() && !drivetrainNeedsStopped) {
    float error = normalize180(heading - getHeading());
    float output = turnPID.update(error);
    output = threshold(output, -turnMaxVoltage, turnMaxVoltage);
    driveWithVoltage(output, -output);
    wait(10, msec);
  }
  if (earlyExitFactor == 1)
  {
    leftDrive.stop(hold);
    rightDrive.stop(hold);
  }
}

void Drive::driveDistance(float distance) {
  driveDistance(distance, driveMaxVoltage, targetHeading, headingMaxVoltage);
}

void Drive::driveDistance(float distance, float driveMaxVoltage) {
  driveDistance(distance, driveMaxVoltage, targetHeading, headingMaxVoltage);
}

void Drive::driveDistance(float distance, float heading, float earlyExitFactor)
{
  driveDistance(distance, driveMaxVoltage, heading, headingMaxVoltage, earlyExitFactor);
}

void Drive::driveDistance(float distance, float driveMaxVoltage, float heading, float headingMaxVoltage,  float earlyExitFactor)
{
  if (earlyExitFactor > 10) earlyExitFactor = 10;
  if (earlyExitFactor < 1) earlyExitFactor = 1;
  targetHeading = normalize360(heading);
  PID drivePID(driveKp, driveKi, driveKd, driveStarti, driveSettleError * earlyExitFactor, driveSettleTime/earlyExitFactor, driveTimeout);
  PID headingPID(headingKp, headingKd);
  float startAveragePosition = (getLeftPosition() + getRightPosition()) / 2.0;
  float averagePosition = startAveragePosition;
  while (drivePID.isDone() == false && !drivetrainNeedsStopped) {
    averagePosition = (getLeftPosition() + getRightPosition()) / 2.0;
    float driveError = distance + startAveragePosition - averagePosition;
    float headingError = normalize180(targetHeading - getHeading());
    float driveOutput = drivePID.update(driveError);
    float headingOutput = headingPID.update(headingError);

    driveOutput = threshold(driveOutput, -driveMaxVoltage, driveMaxVoltage);
    headingOutput = threshold(headingOutput, -headingMaxVoltage, headingMaxVoltage);

    driveWithVoltage(driveOutput + headingOutput, driveOutput - headingOutput);
    wait(10, msec);
  }
  if (earlyExitFactor == 1)
  {
    leftDrive.stop(hold);
    rightDrive.stop(hold);
  }
}

void Drive::setArcadeConstants(float kBrake, float kTurnBias, float kTurnDampingFactor)
{
  this->kBrake = kBrake;
  this->kTurnBias = kTurnBias;
  this->kTurnDampingFactor = kTurnDampingFactor;
}

void Drive::controlArcade(int y, int x) {
  float throttle = deadband(y, 5);
  float turn = deadband(x, 5) * kTurnDampingFactor;

  turn = curveFunction(turn, kTurn);
  throttle = curveFunction(throttle, kThrottle);

  float leftPower = percentToVolt(throttle + turn);
  float rightPower = percentToVolt(throttle - turn);

  if (kTurnBias > 0) {
    if (fabs(throttle) + fabs(turn) > 100) {
      int oldThrottle = throttle;
      int oldTurn = turn;
      throttle *= (1 - kTurnBias * fabs(oldTurn / 100.0));
      turn *= (1 - (1 - kTurnBias) * fabs(oldThrottle / 100.0));
    }
    leftPower = percentToVolt(throttle + turn);
    rightPower = percentToVolt(throttle - turn);
  }

  if (fabs(throttle) > 0 || fabs(turn) > 0) {
    leftDrive.spin(fwd, leftPower, volt);
    rightDrive.spin(fwd, rightPower, volt);
    drivetrainNeedsStopped = true;
  }
  // When joystick are released, run active brake on drive
  // ajdust the coefficient to the amount of coasting preferred
  else {
    if (drivetrainNeedsStopped) {
      if (stopMode != hold) {
        leftDrive.resetPosition();
        rightDrive.resetPosition();
        wait(20, msec);
        leftDrive.spin(fwd, -leftDrive.position(rev) * kBrake, volt);
        rightDrive.spin(fwd, -rightDrive.position(rev) * kBrake, volt);
      } else {
        leftDrive.stop(hold);
        rightDrive.stop(hold);
      }
      drivetrainNeedsStopped = false;
    }
  }
}

void Drive::controlTank(int left, int right) {
  float leftthrottle = curveFunction(left, kThrottle);
  float rightthrottle = curveFunction(right, kThrottle);

  if (fabs(leftthrottle) > 0 || fabs(rightthrottle) > 0) {
    leftDrive.spin(fwd, percentToVolt(leftthrottle), volt);
    rightDrive.spin(fwd, percentToVolt(rightthrottle), volt);
    drivetrainNeedsStopped = true;
  } else {
    if (drivetrainNeedsStopped) {
      leftDrive.stop(stopMode);
      rightDrive.stop(stopMode);
      drivetrainNeedsStopped = false;
    }
  }
}

void Drive::controlMecanum(int x, int y, int acc, int steer, motor DriveLF, motor DriveRF, motor DriveLB, motor DriveRB) {
  float throttle = deadband(y, 5);
  float strafe = deadband(x, 5);
  float straight = deadband(acc, 5);
  float turn = deadband(steer, 5);
  straight = curveFunction(straight, kThrottle);
  turn = curveFunction(turn, kTurn);

  if (turn == 0 && strafe == 0 && throttle == 0 && straight == 0) {
    if (drivetrainNeedsStopped) {
      leftDrive.stop(stopMode);
      rightDrive.stop(stopMode);
      drivetrainNeedsStopped = false;
      return;
    }
  } 

  if (turn == 0 && straight == 0) {
    DriveLF.spin(fwd, percentToVolt(throttle + turn + strafe), volt);
    DriveRF.spin(fwd, percentToVolt(throttle - turn - strafe), volt);
    DriveLB.spin(fwd, percentToVolt(throttle + turn - strafe), volt);
    DriveRB.spin(fwd, percentToVolt(throttle - turn + strafe), volt);
  }
  //aracade drive
  else
  {
    float leftPower = percentToVolt(throttle + turn);
    float rightPower = percentToVolt(throttle - turn);
    leftDrive.spin(fwd, leftPower, volt);
    rightDrive.spin(fwd, rightPower, volt);
    drivetrainNeedsStopped = true;
  }
}

void Drive::stop(vex::brakeType mode) {
  drivetrainNeedsStopped = true;
  leftDrive.stop(mode);
  rightDrive.stop(mode);
  stopMode = mode;
  leftDrive.resetPosition();
  rightDrive.resetPosition();
  drivetrainNeedsStopped = false;
}

void Drive::checkStatus(){
  int distanceTraveled = (getLeftPosition() + getRightPosition()) / 2.0;
    // Display heading and the distance traveled previously on the controller screen.
  int h = inertialSensor.rotation();
  char statusMsg[50];
  sprintf(statusMsg, "heading: %d, dist: %d", h, distanceTraveled);
  printControllerScreen(statusMsg);
}

 void  Drive::driveToWall(float targetWallDistance)
 {
    driveToWall(targetWallDistance, driveMaxVoltage, targetHeading, headingMaxVoltage);
 }

  void  Drive::driveToWall(float targetWallDistance, float targetHeading, float earlyExitFactor)
 {
    driveToWall(targetWallDistance, driveMaxVoltage, targetHeading, headingMaxVoltage, earlyExitFactor);
 }

 void  Drive::driveToWall(
    float targetWallDistance, 
    float driveMaxVoltage,
    float heading,
    float headingMaxVoltage,
    float earlyExitFactor,
    float denoise){
  
  if (getDistanceFunc == nullptr) return; // no distance function defined
  
  if (earlyExitFactor > 10) earlyExitFactor = 10;
  if (earlyExitFactor < 1) earlyExitFactor = 1;
  targetHeading = normalize360(heading);
  PID drivePID(driveKp, driveKi, driveKd, driveStarti, driveSettleError * earlyExitFactor, driveSettleTime/earlyExitFactor, driveTimeout);
  PID headingPID(headingKp, headingKd);
  float d, prevD;
  prevD = getDistanceFunc();
  if (prevD == 0) prevD = targetWallDistance;
  float alpha = 1 - denoise; // low-pass filter factor
  while (drivePID.isDone() == false && !drivetrainNeedsStopped) {
    d = getDistanceFunc();
    if (d == 0) d = targetWallDistance;  // safety clamp
    // Low-pass filtered distance reading to smooth data
    d = alpha * d + (1 - alpha) * prevD;
    prevD = d;

    float driveError = d - targetWallDistance;
    float headingError = normalize180(targetHeading - getHeading());
    float driveOutput = drivePID.update(driveError);
    float headingOutput = headingPID.update(headingError);
    driveOutput = threshold(driveOutput, -driveMaxVoltage, driveMaxVoltage);
    headingOutput = threshold(headingOutput, -headingMaxVoltage, headingMaxVoltage);
    driveWithVoltage(driveOutput + headingOutput, driveOutput - headingOutput);
   
    wait(10, msec);
  }
  if (earlyExitFactor == 1)
  {
    leftDrive.stop(hold);
    rightDrive.stop(hold);
  }
}