#include "vex.h"

using namespace vex;
brain Brain;
controller controller1 = controller(primary);

// ------------------------------------------------------------------------
//              Drivetrain definition
// ------------------------------------------------------------------------
//If you only have 4  motors (or mecanum drive), assign leftMotor3, rightMotor3 to unused ports.
motor leftMotor1 = motor(PORT11, ratio18_1, true);
motor leftMotor2 = motor(PORT12, ratio6_1, true);
motor leftMotor3 = motor(PORT13, ratio6_1, true);

motor rightMotor1 = motor(PORT5, ratio18_1, false);
motor rightMotor2 = motor(PORT2, ratio6_1, false);
motor rightMotor3 = motor(PORT3, ratio6_1, false);

inertial inertial1 = inertial(PORT7);
float inertialSensorScale = 360;

// ------------------------------------------------------------------------
//        Other subsystems: motors, sensors and helper functions definition
// ------------------------------------------------------------------------
motor intakeMotor = motor(PORT17, ratio6_1, false);
motor scoreMotor = motor(PORT18, ratio6_1, false);

motor matchLoadMotor = motor(PORT14, ratio18_1, false);
motor wingMotor = motor(PORT6, ratio18_1, false);

// total number of motors, including drivetrain
const int NUMBER_OF_MOTORS = 10;

distance frontDistanceSensor = distance(PORT4);
optical colorSortOptical = optical(PORT19); // assign to an unused port if not used
optical matchLoadOptical = optical (PORT10); // assign to an unused port if not used

float getFrontDistance(){
  float d = 0;
  if(frontDistanceSensor.installed()){
    d = frontDistanceSensor.objectDistance(inches);
  }
  if(d < 10 || d > 144){
    d = 0;
  }
  return d;
}

void intake(){
  intakeMotor.spin(forward, 12, volt);
  scoreMotor.stop(hold);
  scoreMotor.spin(forward, 1, volt);
}

void stopRollers(){
  intakeMotor.stop(brake);
  scoreMotor.stop(brake);
  chassis.stop(coast);
}

void reverseIntake(){
  chassis.stop(hold);
  intakeMotor.spin(reverse, 12, volt);
  scoreMotor.spin(reverse, 12, volt);
}

void scoreLong(){
  intakeMotor.spin(forward, 12, volt);
  scoreMotor.spin(forward, 12, volt);
  chassis.stop(hold);
}

void scoreMiddle(){
  intakeMotor.spin(forward, 12, volt);
  scoreMotor.spin(forward, 30, pct);
  chassis.stop(hold);
}

void ejectBalls(){
  intakeMotor.spin(forward, 12, volt);
  scoreMotor.spin(reverse, 70, pct); 
}

bool matchColor(){
  if (colorSortMode == 0 || !colorSortOptical.installed()) {
    return true; // color sorting disabled
  }
  colorSortOptical.setLight(ledState::on);
  colorSortOptical.setLightPower(100, percent);
  if (!colorSortOptical.isNearObject()) return true; // no object detected
  // read color from optical sensor
  color r = colorSortOptical.color();
  if ( (r == color::red && colorSortMode == 2) ||  (r == color::blue && colorSortMode == 1))  {
    return false; // wrong color
  }
  
  return true; 
}

bool matchLoadUp = true;
void toggleMatchLoad()
{
  if (matchLoadMotor.installed() == false) return;
  matchLoadMotor.stop(hold);
  matchLoadMotor.setVelocity(100, percent);
  matchLoadMotor.setTimeout(500, msec);

  matchLoadUp = !matchLoadUp;
  if (!matchLoadUp)
  {
    matchLoadMotor.spinFor(forward, 300, degrees);
    matchLoadMotor.stop(coast);
  }
  else
  {
    matchLoadMotor.spinFor(reverse, 300, degrees);
    matchLoadMotor.stop(coast);
    wait(20, msec);
    matchLoadMotor.stop(hold);
  }
}

void setMatchload(bool down)
{
  if (matchLoadMotor.installed() == false) return;

  if (matchLoadUp && down)
  {
    toggleMatchLoad();
    matchLoadUp = false;
    return;
  }
  if (!matchLoadUp && !down)
  {
    toggleMatchLoad();
    matchLoadUp = true;
  }
}

bool wingUp = false;
void setWing(bool descore){
  if (wingMotor.installed() == false) return;
  if(descore && wingUp){
    wingMotor.setVelocity(100, percent);
    wingMotor.setTimeout(300, msec);
    wingMotor.spinFor(reverse, 170, degrees); //tune degree for wing down angle
    wingMotor.stop(brake);
    wingUp = false;
    return;
  }
  if (!descore && !wingUp){
    wingMotor.spin(forward, 6, volt);
    wait(200, msec);
    wingMotor.stop(brake);
    wingUp = true;
  }
}

void scoreBalls(int duration)
{
  int count = 0;
  scoreLong();
  while(count < duration/100) {
    if (matchColor()) scoreLong();
    else ejectBalls();
    wait (100, msec);
    count++;
  }
  stopRollers();
}


void matchLoadUntilColor(int timeout)
{
  bool isOpticalInstalled = false;
  double t = Brain.Timer.time(msec);//original time

  if (matchLoadOptical.installed()){//if not installed then just waits for timeout
    matchLoadOptical.setLight(ledState::on);
    matchLoadOptical.setLightPower(100, percent);
    isOpticalInstalled = true;
  }
  while(Brain.Timer.time(msec) < timeout + t){
    if (isOpticalInstalled){ 
      // read color from optical sensor
      color r = matchLoadOptical.color();
      if ( (r == color::red && colorSortMode == 2) ||  (r == color::blue && colorSortMode == 1))  {
        break; // wrong color
      }
    }
    wait(20, msec);
  }
  stopRollers();
  matchLoadOptical.setLight(ledState::off);
}

void scoreBallsUntilNone(int timeout)
{
  const int WaitDurationMsec    = 100;  // loop cadence
  const int DebounceWindowMsec  = 150;  // require this much continuous "no ball" to stop

  int elapsed     = 0;
  int noBallAccum = 0;

  // If optical sensor not present, fallback to timed scoring
  if (!colorSortOptical.installed()) {
    scoreBalls(timeout);
    return;
  }

  // Main loop: score/eject based on color sorting, stop when sensor reports no ball stably
  while (elapsed < timeout)
  {
    // handle ball: respect color sort logic
    if (!matchColor()) {
      ejectBalls();
    } else {
      scoreLong();
    }

    // advance time and give mechanism time to move the ball
    wait(WaitDurationMsec, msec);
    elapsed += WaitDurationMsec;

    // read optical sensor
    bool ballDetected = colorSortOptical.isNearObject();

    if (!ballDetected) {
      noBallAccum += WaitDurationMsec;
      if (noBallAccum >= DebounceWindowMsec) {
        break; // confirmed empty
      }
    } else {
      // reset debounce and wait until object clears so we don't double-score the same ball
      noBallAccum = 0;
      //while (colorSortOptical.isNearObject() && elapsed < timeout) {
        //wait(WaitDurationMsec, msec);
        //elapsed += WaitDurationMsec;
      //}
    }
  }
  colorSortOptical.setLight(ledState::off);
  stopRollers();
}

void additionalSetup() {
  matchLoadMotor.stop(hold);
  matchLoadMotor.setVelocity(100, percent);
  matchLoadMotor.setTimeout(500, msec);
}

// ------------------------------------------------------------------------
//              Button controls
// ------------------------------------------------------------------------
void buttonL1Action() {
 // setWing(false);
  while(controller1.ButtonL1.pressing()){
    if (controller1.ButtonR2.pressing()) 
    {
      // todo:  score bottom maco
      reverseIntake();
    }
    else intake();
    wait(50, msec);
  }
  stopRollers();
}

void buttonL2Action() {
  while(controller1.ButtonL2.pressing()){
    bool match = matchColor();
    if(!match){
      ejectBalls();
    }
    else {
      if (controller1.ButtonR2.pressing()) 
      {
        // eject balls macro
        ejectBalls();
      }
      else scoreLong();
    }
    wait(100, msec);
  }
  stopRollers();
}

void buttonR2Action()
{
  chassis.stop(hold);
  controller1.rumble(".");
  setWing(true);
  while(controller1.ButtonR2.pressing()){
     wait(100, msec);
  }
  setWing(false);
  chassis.stop(coast);
// toggle wing
/*  
if(wingUp) {
    setWing(true);
    chassis.stop(hold);
  } else {
    setWing(false);
    chassis.stop(coast);
  }   */
}

void buttonR1Action(){
  setMatchload(true);
  intake();
  chassis.stop(hold);

  while(controller1.ButtonR1.pressing()){
    wait(100, msec);
  }
  stopRollers();
  float d = getFrontDistance();
  if ( d > 0 && d < FRONTWALL_DISTANCE + 2) 
  {
    chassis.driveDistance(-10, chassis.getHeading(), 1);
  }
  setMatchload(false);
  chassis.stop(coast);
}

void buttonDownAction()
{
  // disable button if in auton test mode
  if (autonTestMode) return; 

  reverseIntake();
  waitUntil(!controller1.ButtonDown.pressing());
  stopRollers();
}


void buttonUpAction()
{
  // disable button if in auton test mode
  if (autonTestMode) return; 

  while(controller1.ButtonUp.pressing()) {
    if (matchColor()) scoreMiddle();
    else ejectBalls();
    wait(100, msec); // wait for the next ball
  }
  stopRollers();
}

void buttonXAction()
{
  // disable button if in auton test mode b/c auton test mode buttons do the same thing
  if (autonTestMode) return; 

  // macro to descore with wing
  chassis.setHeading(180);

  chassis.driveDistance(11, 180, 2);
  chassis.turnToHeading(120, 10, 5);
  chassis.driveDistance(-10, 135, 2);
  chassis.turnToHeading(-170, 10, 10, -1);
  setWing(true);
  chassis.driveDistance(-16, 180, 1);
  setWing(false);
}

void buttonYAction()
{
  // disable button if in auton test mode b/c auton test mode buttons do the same thing
  if (autonTestMode) return; 

  // macro to matchload and intake
}

void buttonLeftAction(){
  if (autonTestMode) return; 
  changeColorSortMode();

  wait(300, msec);
  if (controller1.ButtonLeft.pressing()){
    chassis.setHeading(180);
    pushWithWing();
  }

}

void buttonRightAction(){
  if (autonTestMode) return; 

  toggleMatchLoad();
}


// ------------------------------------------------------------------------
//               Robot configuration
// ------------------------------------------------------------------------

bool macroMode = false;

// 0: double arcade drive, 1: single aracde, 2: tank drive
// -1: disable drive
int DRIVE_MODE = 0;

void setupButtonMapping() {
  controller1.ButtonL1.pressed(buttonL1Action);
  controller1.ButtonL2.pressed(buttonL2Action);
  controller1.ButtonR2.pressed(buttonR2Action);
  controller1.ButtonR1.pressed(buttonR1Action);
  controller1.ButtonLeft.pressed(buttonLeftAction);
  controller1.ButtonUp.pressed(buttonUpAction);
  controller1.ButtonDown.pressed(buttonDownAction);
  controller1.ButtonRight.pressed(buttonRightAction);

  controller1.ButtonA.pressed(buttonAAction);
  controller1.ButtonY.pressed(buttonYAction);
  controller1.ButtonX.pressed(buttonXAction);
  controller1.ButtonB.pressed(buttonBAction);
}

Drive chassis(
  //Left Motors:
  motor_group(leftMotor2, leftMotor1, leftMotor3),
  //Right Motors:
  motor_group(rightMotor2, rightMotor1, rightMotor3),
  //Inertial Sensor:
  inertial1,
  //wheel diameter:
  2.75,
  //Gear ratio of motor to wheel: if your motor has an 36-tooth gear and your wheel has a 48-tooth gear, this value will be 0.75.
  0.75
);


// Resets the chassis constants.
void setChassisDefaults() {
  // Sets the targeting heading of the chassis to the current heading of the inertial sensor.
  chassis.setGyroScale(inertialSensorScale);
  chassis.setHeading(chassis.inertialSensor.heading());

  chassis.setMaxVoltage(10, 10, 6);
  // Sets the drive PID constants for the chassis.
  // These constants are used to control the acceleration and deceleration of the chassis.
  chassis.setDrivePID(1.5, 0, 10, 0);
  // Sets the turn PID constants for the chassis.
  // These constants are used to control the turning of the chassis.
  chassis.setTurnPID(0.2, .015, 1.5, 7.5);
  // Sets the heading PID constants for the chassis.
  // These constants are used to control the heading adjustment of the chassis.
  chassis.setHeadingPID(0.4, 1);
  // Sets the exit conditions for the drive functions.
  // These conditions are used to determine when the drive function should exit.
  chassis.setDriveExitConditions(1, 300, 2000);
  // Sets the exit conditions for the turn functions.
  // These conditions are used to determine when the turn function should exit.
  chassis.setTurnExitConditions(1.5, 300, 1500);

  // Sets the arcade drive constants for the chassis.
  // These constants are used to control the arcade drive of the chassis.
  chassis.setArcadeConstants(0.5, 0.5, 0.85);
}

void changeDriveMode(){
  controller1.rumble("-");
  DRIVE_MODE = (DRIVE_MODE +1)%3;
    switch (DRIVE_MODE) {
    case 0:
      printControllerScreen("Double Arcade");
      break;
    case 1:
      printControllerScreen("Single Arcade");
      break;
    case 2:
      printControllerScreen("Tank Drive");
      break;
    case 3:
      printControllerScreen("Mecanum Drive");
      break;
    }
}

int colorSortMode = 0; // 0: off, 1: red, 2: blue
void changeColorSortMode() {
  controller1.rumble("-");
  colorSortMode = (colorSortMode + 1) % 3;
  if (colorSortMode == 0) printControllerScreen("No color sort");
  else if (colorSortMode == 1) printControllerScreen("Sort red");
  else if (colorSortMode == 2) printControllerScreen("Sort blue");
}


// This is the user control function.
// It is called when the driver control period starts.
void usercontrol(void) {
  // Exits the autonomous menu.
  exitAuton();
  additionalSetup();
  if (!wingUp) setWing(false);

  // This loop runs forever, controlling the robot during the driver control period.
  while (1) {
    switch (DRIVE_MODE) {
    case 0: // double arcade
      chassis.controlArcade(controller1.Axis2.position(), controller1.Axis4.position());
      break;
    case 1: // single arcade
      chassis.controlArcade(controller1.Axis3.position(), controller1.Axis4.position());
      break;
    case 2: // tank drive
      chassis.controlTank(controller1.Axis3.position(), controller1.Axis2.position());    
      break;
    }

    // This wait prevents the loop from using too much CPU time.
    wait(20, msec);
   } 
}


bool brakeOn = false;
void buttonBAction(){
  // toggle brake
  if (brakeOn) 
  {
    chassis.stop(coast);
    printControllerScreen("coast");
  }
  else 
  {
    chassis.stop(hold);
    printControllerScreen("hold");
  }
  controller1.rumble(".");
  wait(800, msec);
  int h = chassis.inertialSensor.rotation();
  float frontd = getFrontDistance();
  char statusMsg[50];
  sprintf(statusMsg, "h:%d,  f:%.1f", h, frontd);
  printControllerScreen(statusMsg);

  brakeOn = !brakeOn;
}


// ------------------------------------------------------------------------
//              quick test code
// ------------------------------------------------------------------------

void buttonAAction()
{
  // disable button if in auton test mode, config mode, or in compeitition mode
  if (autonTestMode  || endGameTimerEnabled) return; 
  wait(300, msec);
  if (controller1.ButtonA.pressing()) return; //only trigger single press

  double t1 = Brain.Timer.time(sec);
  printControllerScreen("Running test...");

  chassis.setHeading(180);



  double t2 = Brain.Timer.time(sec);
  char timeMsg[30];
  sprintf(timeMsg, "run time: %.1f", t2-t1);
  printControllerScreen(timeMsg);
  stopRollers();
  chassis.stop(coast);
}