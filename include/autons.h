#pragma once
#include "rgb-template/drive.h"
#include "parameters.h"

// helper functions
void pushWithHorn();
void pushWithHood();
void toSideWall(int side);
void toMatchload();
void toLongGoal();
void right4();
void right7();

// todo: actual autons


// system functions and variables
void pre_auton();
void autonomous();
void exitAuton();

void registerAutonTestButtons();

extern int currentAutonSelection;
extern bool autonTestMode;
extern bool endGameTimerEnabled;