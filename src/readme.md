# Button Control
## L1
*  Hold to intake balls
*  Release to stop the roller immediately
*  If hold R2 before press L1, reverse intake

## L2
*  Hold to score long goal
*  Release to stop the roller immediately
*  If hold R2 before press L1, eject the balss

## R2
*  Press and hold to push wing down 
*  Release to raise wing up

## R1
* Press and hold put matchload down and run the intake
* Release to drive backward, raise the matchload, and stop intake

## B
*  toggle brake mode
*  shows heading and distance sensor reading

## A
* Activate test mode if pressed and hold within 5 seconds of starting the program
* If in test mode, press to run the selected autonomous routine or current step for testing
* If not in test mode, press to run quick test code

## Right 
* If in test mode, press to scroll forward through auton menu
* If not in test mode, toggle matchload

## Left 
* press to cycle through color sort modes

## Up
* hold to run score roller half speed, e.g. middle goal

## Down
* hold to reverse intake

## X
* If not in test mode, descore macro from the long goal

# Test mode
Button A = press within 5 sec of start to active test mode
Right arrow in test mode = go to the next auton
A button in test mode = run auton. 

# For comp switch
To change autons, press top half screen. To change color sort, press bottom half.

# Setup program
Right arrow to cycle through default auton
Left arrow to cycle through team color 

# API 
- heading
```cpp
// Sets the current heading of the robot. Always remember to set the heading before testing any auton sequences
  chassis.setHeading(heading);

// Turn to heading with default speed and full stop
  chassis.turnToHeading(heading);

// Turn to heading with voltage, usually for slowed speed
  chassis.turnToHeading(heading, voltage);

// Turn to heading with voltage and not complete stop
  chassis.turnToHeading(heading, voltage, earlyExitFactor);

// Turn to heading while holding the left or right drivepod -1, 1
  chassis.turnToHeading(heading, voltage, earlyExitFactor, swing);
```

- Driving
```cpp
// drive specified distance with default speed and full stop
  chassis.driveDistance(distance)

// drive specified distance with default speed, usually slower, with full stop
  chassis.driveDistance(distance, voltage)

// drive specified distance while changing head, e.g. curving, with optional early exit 
  chassis.driveDistance(distance, heading, earlyExitFactor)

// drive specified distance while changing head, e.g. curving, with specific speed with optional early exit 
  chassis.driveDistance(distance, driveVoltage, heading, headingVoltage, earlyExitFactor)
```

- Distance sensor driving
```cpp
// before calling the driveToWall function, need the set with getdistance function.
  chassis.getDistanceFunc = getFrontDistance;
// drive to the wall until the specified distance. If driving backward, the distance should be negative number
  chassis.driveToWall(targetWallDistance,  targetHeading, earlyExitFactor);
```
