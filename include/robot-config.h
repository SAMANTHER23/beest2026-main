using namespace vex;

void intake();
void stopRollers();
void reverseIntake();
void scoreMiddle();
void scoreLong();
bool matchColor();


void scoreBalls(int duration);  //for three balls
float getFrontDistance();
void setHorn(bool descore);
void setMatchload(bool down);

extern motor scoreMotor;
extern bool matchLoadUp;


// ------------------------------------------------------------------------
//              Standard setup
// ------------------------------------------------------------------------

// A global instance of the brain used for printing to the V5 Brain screen.
extern brain Brain;
extern controller controller1;
// Forward declaration of the Drive class.
class Drive;
// A global instance of the Drive class.
extern Drive chassis;
extern int DRIVE_MODE;
extern int colorSortMode;
extern bool macroMode;
extern const int NUMBER_OF_MOTORS;

void changeDriveMode();
void changeColorSortMode();

void setChassisDefaults();
void additionalSetup();
void usercontrol();

void setupButtonMapping();

void buttonL1Action();
void buttonL2Action();
void buttonR1Action();
void buttonR2Action();
void buttonLeftAction();
void buttonRightAction();
void buttonUpAction();
void buttonDownAction();
void buttonAAction();
void buttonBAction();
void buttonXAction();
void buttonYAction();

