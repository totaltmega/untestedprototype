/*
 * Copyright (c) 2012 Baseflight U.P.
 * Licensed under the MIT License
 * @author  Scott Driessens v0.1 (August 2012)
 */
 
#pragma once

#include "sensors/sensors.h"

///////////////////////////////////////////////////////////////////////////////
// External Variables
///////////////////////////////////////////////////////////////////////////////

extern int16_t rawGyro[3];

extern int16_t rawGyroTemperature;

extern gyro_t *gyro;

///////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////

void readGyro(void);

void readGyroTemp(void);

void computeGyroTCBias(void);

void computeGyroRTBias(void);

void gyroTempCalibration(void);

///////////////////////////////////////////////////////////////////////////////