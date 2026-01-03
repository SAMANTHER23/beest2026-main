#include "vex.h"

int currentAutonSelection = 0;        // Current auton selection

// ----------------------------------------------------------------------------
//             Helper functions
// ----------------------------------------------------------------------------

// assume the robot is facing south
void goaltoMatchLoad(int side) // left side and right side may be different
{
  if(matchLoadUp) 
  {
    setMatchload(true);
    wait(1000, msec);
  }
  intake();

  // drive near  the match load 
  float d = getFrontDistance();
  float h = 180;
  if (side == -1) h = 177;
  if (side == 1) h = 182;

  chassis.driveDistance(d-FRONTWALL_DISTANCE - 2, h, 2);

  // push into the match load
  chassis.getDistanceFunc = getFrontDistance;
  if (side == -1) chassis.driveToWall(FRONTWALL_DISTANCE-4, 6, 180, 6, 3);
  if (side == 1) chassis.driveToWall(FRONTWALL_DISTANCE-3, 6, 180, 6, 3);
  chassis.driveWithVoltage(8, 8);
  wait(100, msec);
  chassis.stop(hold);

  // get matchloads
  matchLoadUntilColor(1500);
//  stopRollers();
}

// assume the robot is either facing left or right
void toSideWall(int side)
{
  chassis.getDistanceFunc = getFrontDistance;
  chassis.driveToWall(SIDEWALL_DISTANCE, 90*side, 2);
  chassis.turnToHeading(180, 10, 2);
}

// assume robot facing south
void toLongGoal()
{
  float d = getFrontDistance();
  chassis.driveDistance(d- GOAL_DISTANCE - 2, 180, 2);
  chassis.stop(hold);
}

// assume at the back of the long goal
void pushWithHood(){
  chassis.driveWithVoltage(10, 10);
  wait(200, msec);
  chassis.stop(brake);
  wait(100, msec); //no tipping
  chassis.driveWithVoltage(-12, -12);
  wait(250, msec);
  chassis.stop(hold); 
}

// assume robot at the back of the long goal and wing is up
void pushWithWing()
{
  chassis.driveDistance(15, 210, 5);
  chassis.turnToHeading(140, 10, 10);
  chassis.driveDistance(-19, 165, 5);
  chassis.turnToHeading(180, 10, 10);
  setWing(true);
  chassis.driveDistance(-19, 6);
}

// ----------------------------------------------------------------------------
//             autons functions
// ----------------------------------------------------------------------------

void left4()
{
  chassis.setHeading(0);
  intake();
  chassis.driveDistance(15, -45, 10);
  chassis.driveDistance(27, 3);

  // drive to long goal
  chassis.turnToHeading(0, 10, 10);
  chassis.driveDistance(-18, 0, 10);
  chassis.turnToHeading(-90, 12, 10);
  toSideWall(-1);
  toLongGoal();
}

void left7()
{
  left4();

  // score balls
  scoreLong();
  setMatchload(true);
  wait(500, msec);
  scoreBallsUntilNone(1000);
  stopRollers();
  
  //get match loads
  intake();
  goaltoMatchLoad(-1);

  //score match loads
  toLongGoal();
  scoreLong();
  setMatchload(false);
  wait(500, msec);
  scoreBallsUntilNone(1000);
  pushWithHood();
}

void leftmatch()
{
  chassis.setHeading(-90);
  toSideWall(-1);
  chassis.turnToHeading(180);
  goaltoMatchLoad(-1);
  matchLoadUntilColor(2000);
  toLongGoal();
  setMatchload(false);
  scoreLong();
  wait(500, msec);
  scoreBallsUntilNone(2000);
  chassis.driveDistance(13, 6);
  chassis.turnToHeading(90);
  chassis.driveDistance(90, 6);
  chassis.turnToHeading(180);
  goaltoMatchLoad(1);
  matchLoadUntilColor(2000);
  toLongGoal();
  scoreLong();
  wait(500, msec);
  scoreBallsUntilNone(2000);
  pushWithHood();
}

void right4()
{
  //get three balls from the right side
  chassis.setHeading(0);
  intake();
  chassis.driveDistance(15, 47, 10);
  chassis.driveDistance(27, 3);

  // drive to long goal
  chassis.turnToHeading(0, 10, 10);
  chassis.driveDistance(-24, 0, 10);
  chassis.turnToHeading(90, 12, 10);
  toSideWall(1);
  toLongGoal();
}

void rightAWP()
{
  right4();
  //score balls in long goal
  scoreLong();
  setMatchload(true);
  wait(500, msec);
  scoreBallsUntilNone(1000);
  stopRollers();

  //get matchload and score at the bottom goal
  goaltoMatchLoad(1);

  // drive back and turn towards middle goal
  chassis.driveDistance(-6.5, 6);
  chassis.turnToHeading(-45, 10);
  setMatchload(false);
  intake();
  wait(200, msec);
  stopRollers();

  // score in bottom goal
  chassis.driveDistance(47.5);
  reverseIntake();
  wait(1500, msec);
  stopRollers();
}

void skillAuton()
{
  intake();
  wait(1000, msec);
  stopRollers();
}

// Runs the selected autonomous routine.
void runAutonItem() {
  additionalSetup();
  switch (currentAutonSelection) {
  case 0:
    left4();
    // score balls
    scoreLong();
    setWing(false);
    wait(500, msec);
    scoreBallsUntilNone(1000);
    stopRollers();
    pushWithWing();
    break;
  case 1:
    left7();
    break;
  case 2:
    right4();
    scoreLong();
    setWing(false);
    wait(500, msec);
    scoreBallsUntilNone(1000);
    stopRollers();
    pushWithWing();
    break;
  case 3:
    rightAWP();
    break;
  case 4:
    skillAuton();
    break;
  }
}

// The names of the autonomous routines to be displayed in the menu.
char const * autonMenuText[] = {
  "left 4",
  "left 7",
  "right 4",
  "right awp",
  "skills"
};


// ----------------------------------------------------------------------------
//                 Menu display
// ----------------------------------------------------------------------------
int autonNum;                         // Total number of autons, automatically calculated based on the size of the autonMenuText array
bool autonTestMode = false;           // Indicates if in test mode
bool exitAutonMenu = false;           // Flag to exit the autonomous menu
bool endGameTimerEnabled = false;      // Flag to indicate if endgame timer is enabled 
const int END_GAME_SECONDS = 85;      // Endgame reminder starts at 85 seconds

// The function is called when the autonomous period starts.
void autonomous(void) {
  // Exits the autonomous menu.
  exitAutonMenu = true;
  endGameTimerEnabled = true;
  // Runs the selected autonomous routine.
  runAutonItem();
}

// This function prints the selected autonomous routine to the brain and controller screens.
void printMenuItem() {
  // Clears the brain screen.
  if (colorSortMode == 1) 
  {
    Brain.Screen.clearScreen(color::red);
  }
  else if (colorSortMode == 2) 
  {
    Brain.Screen.clearScreen(color::blue);
  }
  else
  {
    Brain.Screen.clearScreen(color::black);
  }
  // Sets the cursor to the second row, second column.
  Brain.Screen.setCursor(2, 2);
  currentAutonSelection = currentAutonSelection % autonNum;

  // Prints the selected autonomous routine name.
  Brain.Screen.print("%s", autonMenuText[currentAutonSelection]);
  printControllerScreen(autonMenuText[currentAutonSelection]);
}

// This function displays the autonomous menu on the brain screen.
void showAutonMenu() {
  autonNum = sizeof(autonMenuText) / sizeof(autonMenuText[0]);

  Brain.Screen.setFont(mono40);
  printMenuItem();

  // This loop runs until the autonomous menu is exited.
  while (!exitAutonMenu) {
    if (Brain.Screen.pressing()) {
      // Waits until the finger is lifted up from the screen.
      while (Brain.Screen.pressing()) {
        wait(20, msec);
      }

      if (Brain.Screen.yPosition() < 120)
      {
        // Cycles through the autonomous routines.
        currentAutonSelection = (currentAutonSelection + 1) % autonNum;
        controller1.rumble(".");
      }
      else 
      {
        changeColorSortMode();
      }
      printMenuItem();
    }
    // This wait prevents the loop from using too much CPU time.
    wait(50, msec);
  }
  Brain.Screen.setFont(mono20);
}

// This function is a thread that runs in the background to remind the driver of the end game.
void endgameTimer() {
  // Waits until the end game starts.
  while (Brain.Timer.time(sec) < END_GAME_SECONDS) {
    wait(500, msec);
  }
  if (endGameTimerEnabled)
  {
    printControllerScreen("end game");
    controller1.rumble("-");
  }

  // Checks the motors health every 60 seconds in drive practice
  while(true)
  {
    wait(60, seconds);
    if (!autonTestMode) checkMotors(NUMBER_OF_MOTORS);
  }
}

void exitAuton()
{
  exitAutonMenu = true;
    // Clears the brain timer.
  Brain.Timer.clear();
    // Starts the end game timer thread.
  thread endgameTimer_thread = thread(endgameTimer);
}

bool setupinertialSensor() {
  wait(100, msec);
  if (!chassis.inertialSensor.installed()) {
    printControllerScreen("inertial failure");
    controller1.rumble("---");
    wait(2, seconds);
    return false;  
  }

  chassis.inertialSensor.calibrate(3);
  // Waits until the inertial sensor is calibrated.
  while (chassis.inertialSensor.isCalibrating()) {
    wait(100, msec);
  }
  // Rumbles the controller to indicate that the inertialSensor is calibrated.
  printControllerScreen("gyro calibrated");
  controller1.rumble(".");
  return true;
}

void setupTeamColor(){

}

void loadConfigParameters()
{
  // load parameters from the SD card
  if (Brain.SDcard.isInserted()) {
    if (Brain.SDcard.exists("parameters.txt")) {  
      // open the file for reading
      uint8_t  myReadBuffer[1000];  
      Brain.SDcard.loadfile("parameters.txt", myReadBuffer, sizeof(myReadBuffer));
      wait(0.5, seconds);

      char line_buffer[256];
      char* buffer_ptr = (char*)myReadBuffer;
      char* line_end;

      // Process the buffer line by line
      while ((line_end = strchr(buffer_ptr, '\n')) != NULL) {
          // Copy the current line into the line_buffer
          int line_len = line_end - buffer_ptr;
          strncpy(line_buffer, buffer_ptr, line_len);
          line_buffer[line_len] = '\0';

          // Find the position of the '=' character
          char* equals_sign = strchr(line_buffer, '=');
          if (equals_sign != NULL) {
              // Null-terminate the key part of the string
              *equals_sign = '\0';
              
              // Extract the key and value strings
              char* key = trim_whitespace(line_buffer);
              char* value_str = trim_whitespace(equals_sign + 1);

              // Check if the line is for 'auton' and 'drive_mode'
              if (strcmp(key, "auton") == 0) {
                  currentAutonSelection = atoi(value_str);
              } else if (strcmp(key, "drive_mode") == 0) {
                  DRIVE_MODE = atoi(value_str);
              }
              else if (strcmp(key, "color_sort") == 0) {
                  colorSortMode = atoi(value_str);
                  if(colorSortMode!=0) printControllerScreen(colorSortMode==1?"team red":"team blue");
                  wait(0.5, seconds);
              }
          }
          
          // Move the pointer to the start of the next line
          buffer_ptr = line_end + 1;
      }
    }
  }
}

void saveConfigParameters()
{
  // save parameters to the SD card
  if (Brain.SDcard.isInserted()) {    
    // Create the parameter string with current values
    char parameter_buffer[256];
    sprintf(parameter_buffer, "auton = %d\ndrive_mode = %d\ncolor_sort = %d\n", currentAutonSelection, DRIVE_MODE, colorSortMode);
    
    // Save the parameters to the SD card
    int32_t result = Brain.SDcard.savefile("parameters.txt", (uint8_t*)parameter_buffer, strlen(parameter_buffer));
    
    if (result < 0) {
      printControllerScreen("save failed");
    }
    wait(0.5, seconds);
  } 
}

// This function is called before the autonomous period starts.
void pre_auton() {
  // Sets up the inertialSensor.
  bool inertialSensorSetupSuccess = setupinertialSensor();
  // Sets up the team color.
  setupTeamColor();

  bool motorsSetupSuccess = true;
  // Checks the motors.
  motorsSetupSuccess = checkMotors(NUMBER_OF_MOTORS);
  //set the parameters for the chassis
  setChassisDefaults();

  // load parameters from the SD card
  if (currentAutonSelection!=-1) loadConfigParameters();
  // Shows the autonomous menu.
  if(inertialSensorSetupSuccess && motorsSetupSuccess) 
  {
    showAutonMenu();
    registerAutonTestButtons();
  }
}


// ----------------------------------------------------------------------------
//                 For testing autonomous and configuration
// ----------------------------------------------------------------------------


void autonTestButtonCheck()
{
    // within 5 seconds of driver control.
  while (Brain.Timer.time(sec) < 5 && !autonTestMode) {
    if (controller1.ButtonA.pressing())
    {
      wait(500, msec);
      if (controller1.ButtonA.pressing())
      {
        controller1.rumble("-");
        printControllerScreen("Test mode: ON");
        waitUntil(!controller1.ButtonA.pressing());
        showAutonMenu();
        autonTestMode = true;
        wait(1, seconds);
        break;
      }
    }
    wait(50, msec);
  }
  while(true)
  {
    if (autonTestMode) 
    {
      if(controller1.ButtonY.pressing())
      {
        waitUntil(!controller1.ButtonY.pressing());
        changeColorSortMode();
        saveConfigParameters();
        wait(0.5, sec);
        showAutonMenu();
      }
      if(controller1.ButtonX.pressing())
      {
        waitUntil(!controller1.ButtonX.pressing());
        changeDriveMode();
        saveConfigParameters();
      }
      if(controller1.ButtonRight.pressing())
      {
        waitUntil(!controller1.ButtonRight.pressing());
        controller1.rumble(".");
        //scroll through the auton menu
        currentAutonSelection = (currentAutonSelection + 1) % autonNum;
        saveConfigParameters();
        showAutonMenu();
      }
      if(controller1.ButtonA.pressing() && DRIVE_MODE != -1)
      {
        waitUntil(!controller1.ButtonA.pressing());
        //run the selected autonomous routine for testing and displays the run time.
        controller(primary).rumble(".");
        double t1 = Brain.Timer.time(sec);
        runAutonItem(); 
        double t2 = Brain.Timer.time(sec);
        char timeMsg[30];
        sprintf(timeMsg, "run time: %.1f", t2-t1);
        printControllerScreen(timeMsg);
        chassis.stop(coast);
      }
    } 
    wait(100, msec);
  }
}

// check the controller button press for autonomous testing.
void registerAutonTestButtons()
{
  if (!endGameTimerEnabled)
  {
    thread autonTestButtonThread = thread(autonTestButtonCheck);
  }
}
