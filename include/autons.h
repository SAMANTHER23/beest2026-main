#pragma once
#include "rgb-template/drive.h"
#include "parameters.h"

// helper functions
void pushWithHood();
void pushWithWing();

void toSideWall(int side);
void goaltoMatchLoad(int side = 1);
void toLongGoal();

void left4();
void left5();
void left8();
void left7();

// system functions and variables
void pre_auton();
void autonomous();
void exitAuton();

void registerAutonTestButtons();

extern int currentAutonSelection;
extern bool autonTestMode;
extern bool endGameTimerEnabled;