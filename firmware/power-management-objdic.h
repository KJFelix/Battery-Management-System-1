/* STM32F1 Power Management for Solar Power

This file defines the CANopen object dictionary variables made available to an
external PC and to other processing modules which may be CANopen devices or
tasks running on the same microcontroller.
*/

/*
 * This file is part of the battery-management-system project.
 *
 * Copyright 2013 K. Sarkies <ksarkies@internode.on.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FreeRTOS.h"

#ifndef POWER_MANAGEMENT_OBJDIC_H_
#define POWER_MANAGEMENT_OBJDIC_H_

#define FIRMWARE_VERSION    "1.07a"

#define NUM_BATS    3
#define NUM_LOADS   2
#define NUM_PANELS  1
#define NUM_IFS     NUM_BATS+NUM_LOADS+NUM_PANELS
#define LOAD_1      0
#define LOAD_2      1
#define PANEL       2

/*--------------------------------------------------------------------------*/
/* Battery state identifiers */
/* Type identifies the way the battery is to be charged and the voltage levels involved */
typedef enum {wetT=0, gelT=1, agmT=2} battery_Type;
/* Different battery charge states affecting how they are allocated to load/charger */
typedef enum {normalF=0, lowF=1, criticalF=2, faultyF=3} battery_Fl_States;
/* Operational states identifying current allocation to load/charger */
typedef enum {loadedO=0, chargingO=1, isolatedO=2} battery_Op_States;
/* Stages in the charge cycle */
typedef enum {bulkC=0, absorptionC=1, floatC=2, restC=3, equalizationC=4} battery_Ch_States;
/* Health state: weak - avoid allocating to load, faulty - charging did not end cleanly */
typedef enum {goodH=0, faultyH=1, missingH=2, weakH=3} battery_Hl_States;

/* Represent the measured data arrays in a union as separate or combined */
struct Interface
{
    int16_t battery[NUM_BATS];
    int16_t load[NUM_LOADS];
    int16_t panel[NUM_PANELS];
};

/* These offsets are for battery 1-3, load 1-2 and panel, in order */
union InterfaceGroup
{
    int16_t data[NUM_IFS];
    struct Interface dataArray;
};

/* Battery State structure encapsulates all quantities for a particular battery.
All current, voltage, SoC, charge variables are times 256. */
struct batteryStates {
    uint16_t currentSteady;     /* Time the battery current is unchanging */
    battery_Fl_States fillState;
    battery_Op_States opState;
    battery_Hl_States healthState;
    int16_t lastCurrent;
    int16_t lastVoltage;
    uint16_t SoC;               /* State of Charge is percentage (times 256) */
    int32_t charge;             /* Battery charge is Coulombs (times 256) */
    uint32_t isolationTime;     /* Time that battery is in isolation state */
};

/*--------------------------------------------------------------------------*/
/* Time delays used to pace the scheduling of tasks */

/* The rate at which the watchdog check is updated (1ms ticks) */
#define WATCHDOG_DELAY               ((portTickType)512/portTICK_RATE_MS)

/* The rate at which the charger algorithm is updated (1ms ticks) */
#define CHARGER_DELAY               ((portTickType)512/portTICK_RATE_MS)

/* The rate at which the monitoring is updated (1ms ticks) */
#define MONITOR_DELAY               ((portTickType)512/portTICK_RATE_MS)

/* The default rate at which the samples are taken (1ms ticks) */
#define MEASUREMENT_DELAY           ((portTickType)512/portTICK_RATE_MS)

/* Delay to allow measurements to settle during the calibration sequence (1ms ticks) */
#define CALIBRATION_DELAY           ((portTickType)4096/portTICK_RATE_MS)

/*--------------------------------------------------------------------------*/
/* Calibration factors to convert A/D measurements to physical entities. */

/* For current the scaling factor gives a value in 1/256 Amp precision.
Subtract this from the measured value and scale by this factor.
Then after averaging scale back by 4096 to give the values used here.
Simply scale back further by 256 to get the actual (floating point)
current. Thus the results here are 256 times the current in amperes. */

#define CURRENT_OFFSET 2028

/* Current scale: amplifier gain 10.4 times ref voltage 3.280 times 256 */

#define CURRENT_SCALE 8373

/* Voltage amplifier has different parameters for different board versions */
#if (VERSION==1)

/* Voltage offset: 5 (2 times local ref 2.5V) times the gain 1.803
times 256 times 4096 */

#define VOLTAGE_OFFSET 9453071

/* Voltage scale: amplifier gain 1.803 times ref voltage 3.3 times 256 */

#define VOLTAGE_SCALE 1523

#warning "Version 1 Selected"

#elif (VERSION==2)

/* Voltage offset: 5 (2 times local ref 2.5V) times the gain 1.833
times 256 times 4096 */

#define VOLTAGE_OFFSET 9611946

/* Voltage scale: amplifier gain 1.833 times ref voltage 3.3 times 256 */

#define VOLTAGE_SCALE 1548

#warning "Version 2 Selected"

#elif (VERSION==3)

/* Voltage offset: 5 (2 times local ref 2.5V) times the gain 1.679
times 256 times 4096 */

#define VOLTAGE_OFFSET 10565197

/* Voltage scale: amplifier gain 1.679 times ref voltage 3.3 times 256 */

#define VOLTAGE_SCALE 1418

//#warning "Version 3 Selected"

#else
#error "Version is not defined"
#endif

/* Temperature measurement via LM335 for reference voltage 3.280V.
Scale is 3.28V over 10mV per degree C times 256.
Offset is 2.732V at 0 degrees C over 3.280 times 4096.
Limit is used to prevent charging from occurring in extreme heat. */

#define TEMPERATURE_SCALE   328*256
#define TEMPERATURE_OFFSET  3412
#define TEMPERATURE_LIMIT   45

/*--------------------------------------------------------------------------*/
/* Battery default parameters */

#define BATTERY_CAPACITY_1  100
#define BATTERY_CAPACITY_2  100
#define BATTERY_CAPACITY_3  100
#define BATTERY_TYPE_1      wetT
#define BATTERY_TYPE_2      gelT
#define BATTERY_TYPE_3      wetT
/*--------------------------------------------------------------------------*/
/* Battery Monitoring State default triggers. */

/* These are in absolute voltages times 256. */
#define GOOD_VOLTAGE        3328    /* 13.0V */
#define LOW_VOLTAGE         3072    /* 12.0V */
#define CRITICAL_VOLTAGE    2995    /* 11.5V */
#define WEAK_VOLTAGE        2944    /* 11.1V */

#define LOW_SOC             60*256  /* 60% */
#define CRITICAL_SOC        45*256  /* 45% */

/*--------------------------------------------------------------------------*/
/* Charger algorithm default parameters */

/* Minimum time that the battery is in a rest phase, in seconds. */
#define REST_TIME           30

/* Minimum time that the battery is in the absorption phase, in seconds. */
#define ABSORPTION_TIME     90

/* This defines the lowest the duty cycle is allowed to go as it may not recover
when it needs to be raised. Check that the duty cycle reduction doesn't
cause duty cycle to go to zero at any time. The lower this is, the longer
it will take the duty cycle to rise in response to changes. */
#define MIN_DUTYCYCLE       256

/* Time to wait before passing to float. 2 hours, in seconds. */
#define FLOAT_DELAY         7200

/* SoC above which charging is stopped in float phase (below this it is
assumed that float phase is ended). */
#define FLOAT_BULK_SOC      95*256  /* 95% */

/* SoC minimum setting when battery reaches rest phase. */
#define REST_SoC            70*256  /* 70% */

/* SoC hysteresis value when changing batteries under charge. */
#define SoC_HYSTERESIS      5*256  /* 5% */

/* Number of cycles that a battery in absorption state charge is below the
current limit needed to enter float stage. */
#define FLOAT_DELAY_LIMIT   10
/*--------------------------------------------------------------------------*/
/****** Object Dictionary Items *******/
/* Configuration items, updated externally, are stored to NVM */
/* Values must be initialized in setGlobalDefaults(). */

/*--------------------------------------------------------------------------*/
struct Config
{
/* Valid data block indicator */
    uint8_t validBlock;
/* Communications Control Variables */
    bool enableSend;            /* Any communications transmission occurs */
    bool measurementSend;       /* Measurements are transmitted */
    bool debugMessageSend;      /* Debug messages are transmitted */
/* Recording Control Variables */
    bool recording;             /* Recording of performance data */
/* Battery characteristics and model parameters */
    uint16_t batteryCapacity[NUM_BATS];
    battery_Type batteryType[NUM_BATS];
    int16_t absorptionVoltage[NUM_BATS];
    int16_t floatVoltage[NUM_BATS];
    int16_t floatStageCurrentScale[NUM_BATS];
    int16_t bulkCurrentLimitScale[NUM_BATS];
    int16_t alphaR;         /* forgetting factor for battery resistance estimator */
    int16_t alphaV;         /* forgetting factor for battery voltage smoothing */
    int16_t alphaC;         /* forgetting factor for battery current smoothing */
/* Tracking Control Variables */
    bool autoTrack;             /* Automatic management of batteries */
    uint8_t panelSwitchSetting; /* Global value of the panel switch setting */
    uint8_t monitorStrategy;    /* Bitmap of monitoring strategies (see monitor). */
    int16_t lowVoltage;         /* Low voltage threshold */
    int16_t criticalVoltage;    /* Critical voltage threshold */
    int16_t lowSoC;             /* Low SoC threshold */
    int16_t criticalSoC;        /* Critical SoC threshold */
    int16_t floatBulkSoC;       /* SoC to change from flat back to bulk phase */
/* Charger Control Variables */
    uint8_t chargerStrategy;    /* Bitmap of charging strategies (see charger). */
    int16_t restTime;           /* Minimum time to rest battery under charge */
    uint16_t absorptionTime;    /* Minimum time for battery to stay in absorption */
    int16_t minDutyCycle;       /* Minimum PWM duty cycle */
    int16_t floatTime;          /* Time before battery placed in float phase */
/* Delay Variables */
    portTickType watchdogDelay;
    portTickType chargerDelay;
    portTickType measurementDelay;
    portTickType monitorDelay;
    portTickType calibrationDelay;
/* System Parameters */
    union InterfaceGroup currentOffsets;
};

/* Map the configuration data also as a block of words.
Block size equal to a FLASH page (2048 bytes) to avoid erase problems.
Needed for reading and writing from Flash. */
#define CONFIG_BLOCK_SIZE       2048
union ConfigGroup
{
    uint8_t data[CONFIG_BLOCK_SIZE];
    struct Config config;
};

/*--------------------------------------------------------------------------*/
/* Prototypes */
/*--------------------------------------------------------------------------*/

void setGlobalDefaults(void);
uint32_t writeConfigBlock(void);

void setBatteryChargeParameters(int battery);
battery_Type getBatteryType(int battery);
int16_t getBatteryCapacity(int battery);
int16_t getBulkCurrentLimit(int battery);
int16_t getFloatStageCurrent(int battery);
int16_t getAbsorptionVoltage(int battery);
int16_t getFloatVoltage(int battery);
int16_t getAlphaV(void);
int16_t getAlphaC(void);
int16_t getAlphaR(void);
int16_t getCurrentOffset(uint8_t interface);
void setCurrentOffset(uint8_t interface, int16_t offset);
portTickType getWatchdogDelay(void);
portTickType getChargerDelay(void);
portTickType getMeasurementDelay(void);
portTickType getMonitorDelay(void);
portTickType getCalibrationDelay(void);
uint8_t getPanelSwitchSetting(void);
void setPanelSwitchSetting(uint8_t battery);
bool isRecording(void);
bool isAutoTrack(void);
uint8_t getMonitorStrategy(void);
uint16_t getControls(void);

#endif

