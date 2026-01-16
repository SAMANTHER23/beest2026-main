#pragma once
#include "rgb-template/drive.h"
#include "parameters.h"

// helper functions
void pushWithHood();
void pushWithWing(int distance = 24, int voltage = 6);

void toLongGoal(); 
void sideWallForLongGoal(int side);
void sideWallForMatchload(int side);
void get3Matchloads();
void matchloadToLongGoal(int side);

void field4(int side);
void left7();
void matchload4(int side);

void rightAWP();

void left4right3();
void leftToRight7();

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