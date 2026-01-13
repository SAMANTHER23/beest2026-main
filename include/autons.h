#pragma once
#include "rgb-template/drive.h"
#include "parameters.h"

// helper functions
void pushWithHood();
void pushWithWing();
void setWing(bool down);

void toSideWall(int side);
void get3Matchloads();
void goalToMatchload(int side = 1);
void directToMatchload(int side);
void toLongGoal();

void scoreBottomFromGoal();

void left4();
void left7();
void right4();
void rightAWP();
void skillAuton();

// system functions and variables
void pre_auton();
void autonomous();
void exitAuton();

void registerAutonTestButtons();

extern int currentAutonSelection;
extern bool autonTestMode;
extern bool endGameTimerEnabled;