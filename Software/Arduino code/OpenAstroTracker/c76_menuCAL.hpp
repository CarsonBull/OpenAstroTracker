#pragma once
#include "Configuration_adv.hpp"
#include "b_setup.hpp"

#if HEADLESS_CLIENT == 0

#if SUPPORT_CALIBRATION == 1

#if GYRO_LEVEL == 1
#include "Gyro.hpp"
#endif
// HIGHLIGHT states allow you to pick one of the three sub functions.
#define HIGHLIGHT_FIRST 1
#define HIGHLIGHT_POLAR 1
#define HIGHLIGHT_SPEED 2
#define HIGHLIGHT_DRIFT 3
#define HIGHLIGHT_RA_STEPS 4
#define HIGHLIGHT_DEC_STEPS 5
#define HIGHLIGHT_BACKLASH_STEPS 6

#if AZIMUTH_ALTITUDE_MOTORS == 1
#define HIGHLIGHT_AZIMUTH_ADJUSTMENT 7
#define HIGHLIGHT_ALTITUDE_ADJUSTMENT 8
#if GYRO_LEVEL == 1
#define HIGHLIGHT_ROLL_LEVEL 9
#define HIGHLIGHT_PITCH_LEVEL 10
#define HIGHLIGHT_LAST 10
#else
#define HIGHLIGHT_LAST 8
#endif
#else
#if GYRO_LEVEL == 1
#define HIGHLIGHT_ROLL_LEVEL 7
#define HIGHLIGHT_PITCH_LEVEL 8
#define HIGHLIGHT_LAST 8
#else
#define HIGHLIGHT_LAST 6
#endif
#endif

// Polar calibration goes through these three states:
//  11- moving to RA and DEC beyond Polaris and waiting on confirmation that Polaris is centered
//  13- moving back to home position
#define POLAR_CALIBRATION_WAIT_CENTER_POLARIS 11
#define POLAR_CALIBRATION_WAIT_HOME 12

// Speed calibration only has one state, allowing you to adjust the speed with UP and DOWN
#define SPEED_CALIBRATION 14

// Drift calibration goes through 2 states
// 15- Display four durations and wait for the user to select one
// 16- Start the calibration run after user presses SELECT. This state waits 1.5s, takes duration time
//     to slew east in half the time selected, then waits 1.5s and slews west in the same duration, and waits 1.5s.
#define DRIFT_CALIBRATION_WAIT 15
#define DRIFT_CALIBRATION_RUNNING 16

// RA step calibration only has one state, allowing you to adjust the number of steps with UP and DOWN
#define RA_STEP_CALIBRATION 17

// DEC step calibration only has one state, allowing you to adjust the number of steps with UP and DOWN
#define DEC_STEP_CALIBRATION 18

// Backlash calibration only has one state, allowing you to adjust the number of steps with UP and DOWN
#define BACKLASH_CALIBRATION 19

// Brightness setting only has one state, allowing you to adjust the brightness with UP and DOWN
// #define BACKLIGHT_CALIBRATION 20

// Azimuth adjustment has one state, allowing you to move azimuth a number of minutes
#define AZIMUTH_ADJUSTMENT 20

// Azimuth adjustment has one state, allowing you to move azimuth a number of minutes
#define ALTITUDE_ADJUSTMENT 21

// Roll Offset Calibration only has one state, allowing you to set the current roll angle as level
#define ROLL_OFFSET_CALIBRATION 22

// Pitch Offset Calibration only has one state, allowing you to set the current pitch angle as level
#define PITCH_OFFSET_CALIBRATION 23

// Start off with Polar Alignment higlighted.
byte calState = HIGHLIGHT_FIRST;

// Speed adjustment variable. Added to 1.0 after dividing by 10000 to get final speed
float SpeedCalibration;

// The current delay in ms when changing calibration value. The longer a button is depressed, the smaller this gets.
int calDelay = 150;

// The index of durations that the user has selected.
byte driftSubIndex = 1;

// The requested total duration of the drift alignment run.
byte driftDuration = 0;

// The number of steps to use for backlash compensation (read from the mount).
int BacklashSteps = 0;

// The arc minutes for the adjustment of Azimuth or Altitude
int AzimuthMinutes = 0;
int AltitudeMinutes = 0;

// Pitch and roll offset
#if GYRO_LEVEL == 1
float PitchCalibrationAngle = 0.0;
float RollCalibrationAngle = 0.0;
bool gyroStarted = false;
#endif

#if AZIMUTH_ATLITUDE_MOTORS == 1
bool azAltMotorsEnabled = false;
#endif

// The brightness of the backlight of the LCD shield.
// int Brightness = 255;

void gotoNextMenu()
{
  lcdMenu.setNextActive();

#if AZIMUTH_ALTITUDE_MOTORS == 1
  mount.disableAzAltMotors();
  azAltMotorsStarted = false;
#endif

#if GYRO_LEVEL == 1
  Gyro::shutdown();
  gyroStarted = false;
#endif
}

bool checkProgressiveUpDown(int *val)
{
  bool ret = true;

  if (lcdButtons.currentState() == btnUP)
  {
    *val = *val + 1;
    mount.delay(calDelay);
    calDelay = max(25, 0.94 * calDelay);
    ret = false;
  }
  else if (lcdButtons.currentState() == btnDOWN)
  {
    *val = *val - 1;
    mount.delay(calDelay);
    calDelay = max(25, 0.94 * calDelay);
    ret = false;
  }
  else
  {
    calDelay = 150;
  }

  return ret;
}

// Since the mount persists this in EEPROM and no longer in global
// variables, we need to update it from the mount to globals when
// we are about to edit it.
void gotoNextHighlightState(int dir)
{
  calState = adjustWrap(calState, dir, HIGHLIGHT_FIRST, HIGHLIGHT_LAST);

  if (calState == HIGHLIGHT_RA_STEPS)
  {
    RAStepsPerDegree = mount.getStepsPerDegree(RA_STEPS);
  }
  else if (calState == HIGHLIGHT_DEC_STEPS)
  {
    DECStepsPerDegree = mount.getStepsPerDegree(DEC_STEPS);
  }
  else if (calState == HIGHLIGHT_BACKLASH_STEPS)
  {
    BacklashSteps = mount.getBacklashCorrection();
  }
  else if (calState == HIGHLIGHT_SPEED)
  {
    SpeedCalibration = (mount.getSpeedCalibration() - 1.0) * 10000.0 + 0.5;
  }
#if GYRO_LEVEL == 1
  else if (calState == HIGHLIGHT_PITCH_LEVEL)
  {
    PitchCalibrationAngle = mount.getPitchCalibrationAngle();
    LOGV2(DEBUG_INFO, "CAL: initial pitch is %f", PitchCalibrationAngle);
  }
  else if (calState == HIGHLIGHT_ROLL_LEVEL)
  {
    RollCalibrationAngle = mount.getRollCalibrationAngle();
    LOGV2(DEBUG_INFO, "CAL: initial roll is %f", RollCalibrationAngle);
  }
#endif
  // else if (calState == HIGHLIGHT_BACKLIGHT) {
  //   Brightness = lcdMenu.getBacklightBrightness();
  // }
}

bool processCalibrationKeys()
{
  byte key;
  bool waitForRelease = false;
  bool checkForKeyChange = true;

#if AZIMUTH_ALTITUDE_MOTORS == 1
  if (!azAltMotorsStarted)
  {
    mount.enableAzAltMotors();
    azAltMotorsStarted = true;
  }

#endif

#if GYRO_LEVEL == 1
  if (!gyroStarted)
  {
    Gyro::startup();
    gyroStarted = true;
  }
#endif

  byte currentButtonState = lcdButtons.currentState();
  if (calState == SPEED_CALIBRATION)
  {
    if (currentButtonState == btnUP)
    {
      if (SpeedCalibration < 32760)
      {                        // Don't overflow 16 bit signed
        SpeedCalibration += 1; //0.0001;
        mount.setSpeedCalibration(1.0 + SpeedCalibration / 10000.0, false);
      }

      mount.delay(calDelay);
      calDelay = max(5, 0.96 * calDelay);
      checkForKeyChange = false;
    }
    else if (currentButtonState == btnDOWN)
    {
      if (SpeedCalibration > -32760)
      {                        // Don't overflow 16 bit signed
        SpeedCalibration -= 1; //0.0001;
        mount.setSpeedCalibration(1.0 + SpeedCalibration / 10000.0, false);
      }

      mount.delay(calDelay);
      calDelay = max(5, 0.96 * calDelay);
      checkForKeyChange = false;
    }
    else
    {
      calDelay = 150;
    }
  }
#if AZIMUTH_ALTITUDE_MOTORS == 1
  else if (calState == AZIMUTH_ADJUSTMENT)
  {
    checkForKeyChange = checkProgressiveUpDown(&AzimuthMinutes);
    AzimuthMinutes = clamp(AzimuthMinutes, -60, 60); // Only allow one arc hour at a time. Azimuth range is 2 arc hours
  }
  else if (calState == ALTITUDE_ADJUSTMENT)
  {
    checkForKeyChange = checkProgressiveUpDown(&AltitudeMinutes);
    AltitudeMinutes = clamp(AltitudeMinutes, -60, 60);
  }
#endif
  else if (calState == RA_STEP_CALIBRATION)
  {
    checkForKeyChange = checkProgressiveUpDown(&RAStepsPerDegree);
  }
  else if (calState == DEC_STEP_CALIBRATION)
  {
    checkForKeyChange = checkProgressiveUpDown(&DECStepsPerDegree);
  }
  else if (calState == BACKLASH_CALIBRATION)
  {
    checkForKeyChange = checkProgressiveUpDown(&BacklashSteps);
  }
  // else if (calState == BACKLIGHT_CALIBRATION) {
  //   checkForKeyChange = checkProgressiveUpDown(&Brightness);
  //   if (!checkForKeyChange) {
  //     LOGV2(DEBUG_INFO,"CAL: Brightness changed to %d", Brightness);
  //     Brightness = clamp(Brightness, 0, 255);
  //     LOGV2(DEBUG_INFO,"CAL: Brightness clamped to %d", Brightness);
  //     lcdMenu.setBacklightBrightness(Brightness, false);
  //     LOGV2(DEBUG_INFO,"CAL: Brightness set %d", (int)lcdMenu.getBacklightBrightness());
  //   }
  // }
  else if (calState == POLAR_CALIBRATION_WAIT_HOME)
  {
    if (!mount.isSlewingRAorDEC())
    {
      lcdMenu.updateDisplay();
      calState = HIGHLIGHT_POLAR;
    }
  }
  else if (calState == POLAR_CALIBRATION_WAIT_CENTER_POLARIS){
      #if AZIMUTH_ALTITUDE_MOTORS == 1
      if (currentButtonState == btnUP)
      {
        if (!mount.isRunningALT()) {
          mount.setSpeed(ALTITUDE_STEPS, 500) ;
        }
      }
      else if (currentButtonState == btnDOWN)
      {
        if (!mount.isRunningALT()) {
          mount.setSpeed(ALTITUDE_STEPS, -500) ;
        }
      }
      else if (currentButtonState == btnNONE)
      {
        if (mount.isRunningALT()) {
          mount.setSpeed(ALTITUDE_STEPS, 0) ;
        }
      }
      #endif
  }
  else if (calState == DRIFT_CALIBRATION_RUNNING)
  {
    lcdMenu.setCursor(0, 1);
    lcdMenu.printMenu("Pause 1.5s ...");
    mount.stopSlewing(TRACKING);
    mount.delay(1500);

    lcdMenu.setCursor(0, 1);
    lcdMenu.printMenu("Eastward pass...");
    mount.runDriftAlignmentPhase(EAST, driftDuration);

    lcdMenu.setCursor(0, 1);
    lcdMenu.printMenu("Pause 1.5s ...");
    mount.delay(1500);

    lcdMenu.setCursor(0, 1);
    lcdMenu.printMenu("Westward pass...");
    mount.runDriftAlignmentPhase(WEST, driftDuration);

    lcdMenu.setCursor(0, 1);
    lcdMenu.printMenu("Done. Pause 1.5s");
    mount.delay(1500);
    mount.runDriftAlignmentPhase(0, 0);

    mount.startSlewing(TRACKING);
    calState = HIGHLIGHT_DRIFT;
  }

  if (checkForKeyChange && lcdButtons.keyChanged(&key))
  {
    waitForRelease = true;

    switch (calState)
    {

    case POLAR_CALIBRATION_WAIT_HOME:
    {
      if (key == btnSELECT)
      {
        calState = HIGHLIGHT_POLAR;
      }
      if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_POLAR;
      }
    }
    break;

    case SPEED_CALIBRATION:
    {
      // UP and DOWN are handled above
      if (key == btnSELECT)
      {
        mount.setSpeedCalibration(1.0 + SpeedCalibration / 10000.0, true);
        lcdMenu.printMenu("Speed Stored.");
        mount.delay(500);
        calState = HIGHLIGHT_SPEED;
      }
      else if (key == btnRIGHT)
      {
        mount.setSpeedCalibration(1.0 + SpeedCalibration / 10000.0, true);
        gotoNextMenu();

        calState = HIGHLIGHT_SPEED;
      }
    }
    break;

    case RA_STEP_CALIBRATION:
    {
      // UP and DOWN are handled above
      if (key == btnSELECT)
      {
        mount.setStepsPerDegree(RA_STEPS, RAStepsPerDegree);
        lcdMenu.printMenu("RA steps stored");
        mount.delay(500);
        calState = HIGHLIGHT_RA_STEPS;
      }
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_RA_STEPS;
      }
    }
    break;

    case DEC_STEP_CALIBRATION:
    {
      // UP and DOWN are handled above
      if (key == btnSELECT)
      {
        mount.setStepsPerDegree(DEC_STEPS, DECStepsPerDegree);
        lcdMenu.printMenu("DEC steps stored.");
        mount.delay(500);
        calState = HIGHLIGHT_DEC_STEPS;
      }
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_DEC_STEPS;
      }
    }
    break;

    case BACKLASH_CALIBRATION:
    {
      // UP and DOWN are handled above
      if (key == btnSELECT)
      {
        LOGV2(DEBUG_GENERAL, "CAL Menu: Set backlash to %d", BacklashSteps);
        mount.setBacklashCorrection(BacklashSteps);
        lcdMenu.printMenu("Backlash stored.");
        mount.delay(500);
        calState = HIGHLIGHT_BACKLASH_STEPS;
      }
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_BACKLASH_STEPS;
      }
    }
    break;

#if AZIMUTH_ALTITUDE_MOTORS == 1
    case AZIMUTH_ADJUSTMENT:
    {
      // UP, DOWN, LEFT, and RIGHT are handled above
      if (key == btnSELECT)
      {
        if (AzimuthMinutes == 0)
        {
          calState = HIGHLIGHT_AZIMUTH_ADJUSTMENT;
        }
        else
        {
          mount.moveBy(AZIMUTH_STEPS, 1.0f * AzimuthMinutes);
          AzimuthMinutes = 0;
        }
      }
    }
    break;

    case ALTITUDE_ADJUSTMENT:
    {
      // UP, DOWN, LEFT, and RIGHT are handled above
      if (key == btnSELECT)
      {
        if (AltitudeMinutes == 0)
        {
          calState = HIGHLIGHT_ALTITUDE_ADJUSTMENT;
        }
        else
        {
          mount.moveBy(ALTITUDE_STEPS, 1.0f * AltitudeMinutes);
          AltitudeMinutes = 0;
        }
      }
    }
    break;
#endif

#if GYRO_LEVEL == 1
    case ROLL_OFFSET_CALIBRATION:
    {
      if (key == btnSELECT)
      {
        auto angles = Gyro::getCurrentAngles();
        mount.setRollCalibrationAngle(angles.rollAngle);
        RollCalibrationAngle = angles.rollAngle;
        LOGV2(DEBUG_INFO, "CAL: Set roll to %f", angles.rollAngle);
        calState = HIGHLIGHT_ROLL_LEVEL;
      }
      else if (key == btnLEFT)
      {
        calState = HIGHLIGHT_ROLL_LEVEL;
      }
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_ROLL_LEVEL;
      }
    }
    break;

    case PITCH_OFFSET_CALIBRATION:
    {
      if (key == btnSELECT)
      {
        auto angles = Gyro::getCurrentAngles();
        mount.setPitchCalibrationAngle(angles.pitchAngle);
        PitchCalibrationAngle = angles.pitchAngle;
        LOGV2(DEBUG_INFO, "CAL: Set pitch to %f", angles.pitchAngle);
        calState = HIGHLIGHT_PITCH_LEVEL;
      }
      else if (key == btnLEFT)
      {
        calState = HIGHLIGHT_PITCH_LEVEL;
      }
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_PITCH_LEVEL;
      }
    }
    break;
#endif
      // case BACKLIGHT_CALIBRATION:
      // {
      //   // UP and DOWN are handled above
      //   if (key == btnSELECT) {
      //     LOGV2(DEBUG_GENERAL, "CAL Menu: Set brightness to %d", Brightness);
      //     lcdMenu.setBacklightBrightness(Brightness);
      //     lcdMenu.printMenu("Level stored.");
      //     mount.delay(500);
      //     calState = HIGHLIGHT_BACKLIGHT;
      //   }
      //   else if (key == btnRIGHT) {
      //     gotoNextMenu();
      //     calState = HIGHLIGHT_BACKLIGHT;
      //   }
      // }
      // break;

    case HIGHLIGHT_POLAR:
    {
      if (key == btnDOWN)
        gotoNextHighlightState(1);
      else if (key == btnUP)
        gotoNextHighlightState(-1);
      else if (key == btnSELECT)
      {
        calState = POLAR_CALIBRATION_WAIT_CENTER_POLARIS;

        // Move the RA to that of Polaris. Moving to this RA aligns the DEC axis such that
        // it swings along the line between Polaris and the Celestial Pole.
        mount.targetRA() = DayTime(PolarisRAHour, PolarisRAMinute, PolarisRASecond);

        // Set DEC to move the same distance past Polaris as
        // it is from the Celestial Pole. That equates to 88deg 42' 11.2".
        mount.targetDEC() = DegreeTime(88 - (NORTHERN_HEMISPHERE ? 90 : -90), 42, 11);
        mount.startSlewingToTarget();
      }
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
      }
    }
    break;

    case POLAR_CALIBRATION_WAIT_CENTER_POLARIS:
    {
      if (key == btnSELECT)
      {
        calState = POLAR_CALIBRATION_WAIT_HOME;
        lcdMenu.printMenu("Aligned, homing");
        mount.delay(750);

        // Sync the mount to Polaris, since that's where it's pointing
        DayTime currentRa = mount.currentRA();
        mount.syncPosition(currentRa.getHours(), currentRa.getMinutes(), currentRa.getSeconds(), 89 - (NORTHERN_HEMISPHERE ? 90 : -90), 21, 6);

        // Go home from here
        mount.setTargetToHome();
        mount.startSlewingToTarget();
        gotoNextMenu();
        calState = HIGHLIGHT_POLAR;
      }
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_POLAR;
      }
    }
    break;

    case HIGHLIGHT_SPEED:
    {
      if (key == btnDOWN)
        gotoNextHighlightState(1);
      if (key == btnUP)
        gotoNextHighlightState(-1);
      else if (key == btnSELECT)
        calState = SPEED_CALIBRATION;
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_POLAR;
      }
    }
    break;

    case HIGHLIGHT_DRIFT:
    {
      if (key == btnDOWN)
        gotoNextHighlightState(1);
      if (key == btnUP)
        gotoNextHighlightState(-1);
      else if (key == btnSELECT)
        calState = DRIFT_CALIBRATION_WAIT;
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_POLAR;
      }
    }
    break;

    case DRIFT_CALIBRATION_WAIT:
    {
      if (key == btnDOWN || key == btnLEFT)
      {
        driftSubIndex = adjustWrap(driftSubIndex, 1, 0, 3);
      }
      if (key == btnUP)
      {
        driftSubIndex = adjustWrap(driftSubIndex, -1, 0, 3);
      }
      if (key == btnSELECT)
      {
        // Take off 6s padding time. 1.5s start pause, 1.5s pause in the middle and 1.5s end pause and general time slop.
        // These are the times for one way. So total time is 2 x duration + 4.5s
        int duration[] = {27, 57, 87, 147};
        driftDuration = duration[driftSubIndex];
        calState = DRIFT_CALIBRATION_RUNNING;
      }
      else if (key == btnRIGHT)
      {
        // RIGHT cancels duration selection and returns to menu
        calState = HIGHLIGHT_DRIFT;
        driftSubIndex = 1;
      }
    }
    break;

    case HIGHLIGHT_RA_STEPS:
    {
      if (key == btnDOWN)
        gotoNextHighlightState(1);
      if (key == btnUP)
        gotoNextHighlightState(-1);
      else if (key == btnSELECT)
        calState = RA_STEP_CALIBRATION;
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_FIRST;
      }
    }
    break;

    case HIGHLIGHT_DEC_STEPS:
    {
      if (key == btnDOWN)
        gotoNextHighlightState(1);
      if (key == btnUP)
        gotoNextHighlightState(-1);
      else if (key == btnSELECT)
        calState = DEC_STEP_CALIBRATION;
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_FIRST;
      }
    }
    break;

    case HIGHLIGHT_BACKLASH_STEPS:
    {
      if (key == btnDOWN)
        gotoNextHighlightState(1);
      if (key == btnUP)
        gotoNextHighlightState(-1);
      else if (key == btnSELECT)
        calState = BACKLASH_CALIBRATION;
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_FIRST;
      }
    }
    break;

#if AZIMUTH_ALTITUDE_MOTORS == 1
    case HIGHLIGHT_AZIMUTH_ADJUSTMENT:
    {
      if (key == btnDOWN)
        gotoNextHighlightState(1);
      if (key == btnUP)
        gotoNextHighlightState(-1);
      else if (key == btnSELECT)
        calState = AZIMUTH_ADJUSTMENT;
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_FIRST;
      }
    }
    break;

    case HIGHLIGHT_ALTITUDE_ADJUSTMENT:
    {
      if (key == btnDOWN)
        gotoNextHighlightState(1);
      if (key == btnUP)
        gotoNextHighlightState(-1);
      else if (key == btnSELECT)
        calState = ALTITUDE_ADJUSTMENT;
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_FIRST;
      }
    }
    break;
#endif

#if GYRO_LEVEL == 1
    case HIGHLIGHT_ROLL_LEVEL:
    {
      if (key == btnDOWN)
        gotoNextHighlightState(1);
      if (key == btnUP)
        gotoNextHighlightState(-1);
      else if (key == btnSELECT)
        calState = ROLL_OFFSET_CALIBRATION;
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_FIRST;
      }
    }
    break;

    case HIGHLIGHT_PITCH_LEVEL:
    {
      if (key == btnDOWN)
        gotoNextHighlightState(1);
      if (key == btnUP)
        gotoNextHighlightState(-1);
      else if (key == btnSELECT)
        calState = PITCH_OFFSET_CALIBRATION;
      else if (key == btnRIGHT)
      {
        gotoNextMenu();
        calState = HIGHLIGHT_FIRST;
      }
    }
    break;
#endif

      // case HIGHLIGHT_BACKLIGHT : {
      //   if (key == btnDOWN) gotoNextHighlightState(1);
      //   if (key == btnUP) gotoNextHighlightState(-1);
      //   else if (key == btnSELECT) calState = BACKLIGHT_CALIBRATION;
      //   else if (key == btnRIGHT) {
      //      gotoNextMenu();
      //     calState = HIGHLIGHT_FIRST;
      //   }
      // }
      // break;
    }
  }

  return waitForRelease;
}

void makeIndicator(char *scratchBuffer, float angle)
{
  angle = clamp(angle, -9.9f, 9.9f);
  dtostrf(fabs(angle), 3, 1, &scratchBuffer[8]);
  for (int i = 0; i < 5; i++)
  {
    scratchBuffer[3 + i] = '-';
    scratchBuffer[11 + i] = '-';
  }

  int pos = clamp((int)round(angle * 4), -5, 5);
  if (pos != 0)
  {
    scratchBuffer[pos + ((pos < 0) ? 8 : 10)] = (pos < 0) ? '>' : '<';
  }
}

void printCalibrationSubmenu()
{
  char scratchBuffer[20];
  if (calState == HIGHLIGHT_POLAR)
  {
    lcdMenu.printMenu(">Polar alignment");
  }
  else if (calState == HIGHLIGHT_SPEED)
  {
    lcdMenu.printMenu(">Speed calibratn");
  }
  else if (calState == HIGHLIGHT_DRIFT)
  {
    lcdMenu.printMenu(">Drift alignment");
  }
  else if (calState == HIGHLIGHT_RA_STEPS)
  {
    lcdMenu.printMenu(">RA Step Adjust");
  }
  else if (calState == HIGHLIGHT_DEC_STEPS)
  {
    lcdMenu.printMenu(">DEC Step Adjust");
  }
  else if (calState == HIGHLIGHT_BACKLASH_STEPS)
  {
    lcdMenu.printMenu(">Backlash Adjust");
  }
#if AZIMUTH_ALTITUDE_MOTORS == 1
  else if (calState == HIGHLIGHT_AZIMUTH_ADJUSTMENT)
  {
    lcdMenu.printMenu(">Azimuth Adjst.");
  }
  else if (calState == HIGHLIGHT_ALTITUDE_ADJUSTMENT)
  {
    lcdMenu.printMenu(">Altitude Adjst.");
  }
#endif

#if GYRO_LEVEL == 1
  else if (calState == HIGHLIGHT_ROLL_LEVEL)
  {
    lcdMenu.printMenu(">Roll Offset");
  }
  else if (calState == HIGHLIGHT_PITCH_LEVEL)
  {
    lcdMenu.printMenu(">Pitch Offset");
  }
#endif
  // else if (calState == HIGHLIGHT_BACKLIGHT) {
  //   lcdMenu.printMenu(">LCD Brightness");
  // }
  else if (calState == POLAR_CALIBRATION_WAIT_CENTER_POLARIS)
  {
    if (!mount.isSlewingRAorDEC())
    {
      lcdMenu.setCursor(0, 0);
      lcdMenu.printMenu("Centr on Polaris");
      lcdMenu.setCursor(0, 1);
      lcdMenu.printMenu(">Centered");
    }
  }
  else if (calState == SPEED_CALIBRATION)
  {
    sprintf(scratchBuffer, "SpdFctr: ");
    dtostrf(mount.getSpeedCalibration(), 6, 4, &scratchBuffer[9]);
    lcdMenu.printMenu(scratchBuffer);
  }
  else if (calState == DRIFT_CALIBRATION_WAIT)
  {
    sprintf(scratchBuffer, " 1m  2m  3m  5m");
    scratchBuffer[driftSubIndex * 4] = '>';
    lcdMenu.printMenu(scratchBuffer);
  }
  else if (calState == RA_STEP_CALIBRATION)
  {
    sprintf(scratchBuffer, "RA Steps: %d", RAStepsPerDegree);
    lcdMenu.printMenu(scratchBuffer);
  }
  else if (calState == DEC_STEP_CALIBRATION)
  {
    sprintf(scratchBuffer, "DEC Steps: %d", DECStepsPerDegree);
    lcdMenu.printMenu(scratchBuffer);
  }
  else if (calState == BACKLASH_CALIBRATION)
  {
    sprintf(scratchBuffer, "Backlash: %d", BacklashSteps);
    lcdMenu.printMenu(scratchBuffer);
  }
#if AZIMUTH_ALTITUDE_MOTORS == 1
  else if (calState == AZIMUTH_ADJUSTMENT)
  {
    sprintf(scratchBuffer, "Az: %d arcmins", AzimuthMinutes);
    lcdMenu.printMenu(scratchBuffer);
  }
  else if (calState == ALTITUDE_ADJUSTMENT)
  {
    sprintf(scratchBuffer, "Alt: %d arcmins", AltitudeMinutes);
    lcdMenu.printMenu(scratchBuffer);
  }
#endif
#if GYRO_LEVEL == 1
  else if (calState == ROLL_OFFSET_CALIBRATION)
  {
    auto angles = Gyro::getCurrentAngles();
    sprintf(scratchBuffer, "R: -------------");
    makeIndicator(scratchBuffer, angles.rollAngle - RollCalibrationAngle);
    lcdMenu.printMenu(scratchBuffer);
  }
  else if (calState == PITCH_OFFSET_CALIBRATION)
  {
    auto angles = Gyro::getCurrentAngles();
    sprintf(scratchBuffer, "P: -------------");
    makeIndicator(scratchBuffer, angles.pitchAngle - PitchCalibrationAngle);
    lcdMenu.printMenu(scratchBuffer);
  }
#endif
  // else if (calState == BACKLIGHT_CALIBRATION) {
  //   sprintf(scratchBuffer, "Brightness: %d", Brightness);
  //   lcdMenu.printMenu(scratchBuffer);
  // }
}
#endif

#endif
