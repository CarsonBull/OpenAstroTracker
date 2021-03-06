#pragma once

#include <Arduino.h>
#include <WString.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Some definitions.   ////
//                      ////
//  DO NOT EDIT THESE   ////
////////////////////////////
// Stepper motor types
#define STEP_28BYJ48 0
#define STEP_NEMA17 1
#define ULN2003_DRIVER     0
#define GENERIC_DRIVER     1
#define TMC2209_STANDALONE 2
#define TMC2209_UART       3
//// DO NOT EDIT ABOVE HERE //////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      ////
// STEPPER SETTINGS     ////
//                      ////
////////////////////////////
//
// STEPPER TYPE
#define RA_STEPPER_TYPE   STEP_28BYJ48  //  STEP_28BYJ48  |  STEP_NEMA17
#define DEC_STEPPER_TYPE  STEP_28BYJ48  //  Change according to the steppers you are using
//
//
////////////////////////////
//
// MICROSTEPPING
// Only affects NEMA steppers!
// Only affects calculations, Microstepping is set by MS pins, 
// !!EXCEPT for TMC2209 UART where this value actually sets the SLEW microstepping
#define SET_MICROSTEPPING 8        // 1 | 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256
//                                                        ^-----------------^
//                                                         only if your driver can handle it.
//                                                         TMC2209 can
////////////////////////////
//
// DRIVER SELECTION
// GENERIC drivers include A4988 and any Bipolar STEP/DIR based drivers. Note Microstep assignments in config_pins.
#define RA_DRIVER_TYPE  ULN2003_DRIVER  //  ULN2003_DRIVER  |  GENERIC_DRIVER  |  TMC2209_STANDALONE  |  TMC2209_UART
#define DEC_DRIVER_TYPE ULN2003_DRIVER  //  
//
//
////////////////////////////
//
// TMC2209 UART settings
// These settings work only with TMC2209 in UART connection (single wire to TX)
#define TRACKING_MICROSTEPPING 64  // Set the MS mode for tracking only. Slew MS is set by SET_MICROSTEPPING above

#define RA_RMSCURRENT 1200       // RMS current in mA. Warning: Peak current will be 1.414 times higher!! Do not exceed your steppers max current!
#define RA_STALL_VALUE 100       // adjust this value if the RA autohoming sequence often false triggers, or triggers too late

#define DEC_SLEW_MICROSTEPPING  16  // The microstep mode used for slewing DEC
#define DEC_GUIDE_MICROSTEPPING 64  // The microstep mode used for Guiding DEC only
#define DEC_STALL_VALUE 10    // adjust this value if the RA autohoming sequence often false triggers, or triggers too late
#define DEC_RMSCURRENT 1000   // RMS current in mA. Warning: Peak current will be 1.414 times higher!! Do not exceed your steppers max current!
#define DEC_HOLDCURRENT 20    // [0, ... , 31] x/32 of the run current when standing still. 0=1/32... 31=32/32
#define USE_AUTOHOME 0        // Autohome with TMC2209 stall detection:  ON = 1  |  OFF = 0   
//                  ^^^ leave at 0 for now, doesnt work properly yet
#define RA_AUDIO_FEEDBACK  0 // If one of these are set to 1, the respective driver will shut off the stealthchop mode, resulting in a audible whine
#define DEC_AUDIO_FEEDBACK 0 // of the stepper coils. Use this to verify that UART is working properly. 


////////////////////////////
//
// GUIDE SETTINGS
// This is the multiplier of the normal trackingspeed that a pulse will have 
// NEMA steppers only! Doesnt affect 28BY (which is hardcoded to 2x and 0)
// Note that the East trackingspeed is calculated as the multiplier-1.0
// Standard value: RA 2.0;  DEC 1.0
#define RA_PULSE_MULTIPLIER 1.5
#define DEC_PULSE_MULTIPLIER 1.0


////////////////////////////
//
// INVERT AXIS
// Set to 1 or 0 to invert motor directions
#define INVERT_RA_DIR 0 
#define INVERT_DEC_DIR 0


////////////////////////////
//
// HEMISPHERE
// Set to 1 if you are in the northern hemisphere.
#define NORTHERN_HEMISPHERE 1


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                  ////////
// LCD SETTINGS     ////////
//                  ////////
////////////////////////////
// 
// UPDATE TIME
// Time in ms between LCD screen updates during slewing operations
#define DISPLAY_UPDATE_TIME 200


////////////////////////////
//
// HEADLESS CLIENT
// If you do not have a LCD shield on your Arduino Uno/Mega, set this to 1 on the line below. This is
// useful if you are always going to run the mount from a laptop anyway.
#define HEADLESS_CLIENT 0


////////////////////////////
//
// LCD BUTTON TEST
// Set this to 1 to run a key diagnostic. No tracker functions are on at all.
#define LCD_BUTTON_TEST 0


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     ///
// HARDWARE EXTENSIONS SUPPORT SECTION ///
//                                     ///
//////////////////////////////////////////
//
// Set this to 1 if the mount has motorized Azimuth and Altitude adjustment. Set pins in configuration_pins.hpp
#define AZIMUTH_ALTITUDE_MOTORS  0

#define AZIMUTH_MAX_SPEED 500
#define AZIMUTH_MAX_ACCEL 200
#define AZIMUTH_ARC_SECONDS_PER_STEP (3.99985f)
#define AZIMUTH_STEPS_PER_ARC_MINUTE (60.0f/AZIMUTH_ARC_SECONDS_PER_STEP)

#define ALTITUDE_MAX_SPEED 500
#define ALTITUDE_MAX_ACCEL 200
#define ALTITUDE_ARC_SECONDS_PER_STEP (0.61761f)
#define ALTITUDE_STEPS_PER_ARC_MINUTE (60.0f/ALTITUDE_ARC_SECONDS_PER_STEP)


// Set this to 1 if you are using a NEO6m GPS module for HA/LST and location automatic determination.
// GPS uses Serial1 by default, which is pins 18/19 on Mega. Change in configuration_adv.hpp
#define USE_GPS 0
// If supported, download the library https://github.com/mikalhart/TinyGPSPlus/releases and extract it to C:\Users\*you*\Documents\Arduino\libraries


// Set this to 1 if you are using a MPU6050 electronic level
// Wire the board to 20/21 on Mega. Change in configuration_adv.hpp
#define GYRO_LEVEL 0

// Set this to 1 if your gyro is mounted such that roll and pitch are in the wrong direction
#define GYRO_AXIS_SWAP 1



#if HEADLESS_CLIENT == 0 // <-- Ignore this line
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                         ///
// FEATURE SUPPORT SECTION ///
//                         ///
//////////////////////////////
// Since the Arduino Uno has very little memory (32KB code, 2KB data) all features
// stretch the Uno a little too far. So in order to save memory we allow you to enable 
// and disable features to help manage memory usage.
// If you run the tracker with an Arduino Mega, you can set all the features to 1.
//
// If you would like to drive your OAT mount with only the LCD Shield, or are on a Uno,
// you should set SUPPORT_SERIAL_CONTROL to 0
//
// If you feel comfortable with configuring the OAT at startup manually, you should set
// SUPPORT_GUIDED_STARTUP to 0 (maybe after you've used it for a while you know what to do).
//
// The POI menu can take a little data memory and you may not need it. If not, you can set
// SUPPORT_POINTS_OF_INTEREST to 0
//
//
// Set this to 1 to support full GO menu. 
// If this is set to 0 you still have a GO menu that has Home and Park.
  #define SUPPORT_POINTS_OF_INTEREST   1

// Set this to 1 to support Guided Startup 
  #define SUPPORT_GUIDED_STARTUP       1

// Set this to 1 to support CTRL menu, allowing you to manually slew the mount with the buttons. 
  #define SUPPORT_MANUAL_CONTROL       1

// Set this to 1 to support CAL menu, allowing you to calibrate various things
  #define SUPPORT_CALIBRATION          1

// Set this to 1 to support INFO menu that displays various pieces of information about the mount. 
  #define SUPPORT_INFO_DISPLAY         1

// Set this to 1 to support Serial Meade LX200 protocol support
  #define SUPPORT_SERIAL_CONTROL       1

#endif  // HEADLESS_CLIENT <-- Ignore this    



#if defined(ESP8266) || defined(ESP32) // <-- ignore this line
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                //////////
// WIFI SETTINGS  //////////
//                //////////
////////////////////////////
// These settings are valid only for ESP8266 or ESP32
//
// Define some things, dont change: ///
#define ESPBOARD
#undef HEADLESS_CLIENT
#define HEADLESS_CLIENT 1
#define WIFI_ENABLED 
#if defined(ESP8266) 
  #undef RUN_STEPPERS_IN_MAIN_LOOP
  #define RUN_STEPPERS_IN_MAIN_LOOP 1
#endif
///////////////////////////////////////
//
// SETTINGS
//
  #define INFRA_SSID "YouSSID"
  #define INFRA_WPAKEY "YourWPAKey"
  #define OAT_WPAKEY "superSecret"
  #define HOSTNAME "OATerScope"

  #define WIFI_MODE 2 
  // 0 - Infrastructure Only - Connecting to a Router
  // 1 - AP Mode Only        - Acting as a Router
  // 2 - Attempt Infrastructure, Fail over to AP Mode.
  
#endif // End WIFI SETTINGS


// This is set to 1 for boards that do not support interrupt timers
#define RUN_STEPPERS_IN_MAIN_LOOP 0


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                //////////
// DEBUG OPTIONS  //////////
//                //////////
////////////////////////////
// Debugging output control
// Each bit in the debug level specifies a kind of debug to enable. Do not change.
#define DEBUG_NONE           0x00
#define DEBUG_INFO           0x01
#define DEBUG_SERIAL         0x02
#define DEBUG_WIFI           0x04
#define DEBUG_MOUNT          0x08
#define DEBUG_MOUNT_VERBOSE  0x10
#define DEBUG_GENERAL        0x20
#define DEBUG_MEADE          0x40
#define DEBUG_VERBOSE        0x80
#define DEBUG_ANY            0xFF

////////////////////////////
//
// DEBUG OUTPUT
//
#define DEBUG_LEVEL (DEBUG_NONE)
// #define DEBUG_LEVEL (DEBUG_SERIAL|DEBUG_WIFI|DEBUG_INFO|DEBUG_MOUNT|DEBUG_GENERAL)
// #define DEBUG_LEVEL (DEBUG_ANY)
// #define DEBUG_LEVEL (DEBUG_INFO|DEBUG_MOUNT|DEBUG_GENERAL)
//
// Bit Name                 Output
//  0  DEBUG_INFO           General output, like startup variables and status
//  1  DEBUG_SERIAL         Serial commands and replies
//  2  DEBUG_WIFI           Wifi related output
//  3  DEBUG_MOUNT          Mount processing output
//  4  DEBUG_MOUNT_VERBOSE  Verbose mount processing (coordinates, etc)
//  5  DEBUG_GENERAL        Other misc. output
//  6  DEBUG_MEADE          Meade command handling output
// Set this to specify the amount of debug output OAT should send to the serial port.
// Note that if you use an app to control OAT, ANY debug output will likely confuse that app.
// Debug output is useful if you are using Wifi to control the OAT or if you are issuing
// manual commands via a terminal.
//




////////////////////////////
// ERRORS

#if DEC_HOLDCURRENT < 1 || DEC_HOLDCURRENT > 31
#error "Holdcurrent has to be within 1 and 31!"
#endif
#if RA_RMSCURRENT > 2000 || DEC_RMSCURRENT > 2000
#error "Do you really want to set the RMS motorcurrent above 2 Ampere? Thats almost 3A peak! Delete this error if you know what youre doing" 
#endif
#if RA_STEPPER_TYPE != STEP_28BYJ48 && __AVR_ATmega328P__
#error "Sorry, Arduino Uno does not support NEMA steppers. Use a Mega instead"
#endif



////////////////////////////
// Misc stuff, ignore

#if HEADLESS_CLIENT == 1 
#define SUPPORT_SERIAL_CONTROL 1
#endif

// Set this to 1 this to enable the heating menu
// NOTE: Heating is currently not supported!
#define SUPPORT_HEATING 0

// Make some variables in the sketch files available to the C++ code.
extern bool inSerialControl;
extern String version;
extern byte PolarisRAHour;
extern byte PolarisRAMinute;
extern byte PolarisRASecond;
extern float RAStepsPerRevolution;
extern int RAPulleyTeeth;
extern float RACircumference;
extern float DECStepsPerRevolution;
extern int DecPulleyTeeth;
