#pragma once
#include "rgb-template/drive.h"
#include "parameters.h"

// helper functions
void pushWithHood();
void pushWithWing();

void toLongGoal(); 
void sideWallForLongGoal(int side);
void sideWallForMatchload(int side);
void get3Matchloads();
void matchloadToLongGoal(int side);

void field4(int side);
void matchload4(int side);
void leftFastField4();

void rightAWP();

void get6Matchloads();
void skillAuton();

// system functions and variables
void pre_auton();
void autonomous();
void exitAuton();

void registerAutonTestButtons();

extern int currentAutonSelection;
extern bool autonTestMode;
extern bool endGameTimerEnabled;