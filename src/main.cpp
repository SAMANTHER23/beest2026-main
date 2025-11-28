// vex.h includes all of the headers for the VEX V5 library
#include "vex.h"

// All vex classes and functions are in the vex namespace
using namespace vex;

// A global instance of competition
// This object is used to register callbacks for the autonomous and driver control periods.
competition Competition;


int main() {
  // Register the autonomous and driver control functions.
  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);

  // Run the pre-autonomous function.
  pre_auton();

  // Set up button mapping for the controller
  setupButtonMapping();

  // Prevent main from exiting with an infinite loop.
  while (true) {
    wait(200, msec);
  }
}
