/*
  June 2012

  BaseFlightPlus Rev -

  An Open Source STM32 Based Multicopter

  Includes code and/or ideas from:

  1)AeroQuad
  2)BaseFlight
  3)CH Robotics
  4)MultiWii
  5)S.O.H. Madgwick

  Designed to run on Naze32 Flight Control Board

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "board.h"

///////////////////////////////////////

int16_t rawGyro[3];

int16_t rawGyroTemperature;

gyro_t _gyro;
gyro_t *gyro = &_gyro;

///////////////////////////////////////////////////////////////////////////////
// Read Gyro
///////////////////////////////////////////////////////////////////////////////

void readGyro(void)
{
    gyro->read(rawGyro);
}

void readGyroTemp(void)
{
    gyro->temperature(&rawGyroTemperature);
}

///////////////////////////////////////////////////////////////////////////////
// Compute Gyro Temperature Compensation Bias
///////////////////////////////////////////////////////////////////////////////

void computeGyroTCBias(void)
{
    sensors.gyroTemperature = (float) (rawGyroTemperature + 13200) / 280.0f + 35.0f;

    sensors.gyroTCBias[ROLL] = cfg.gyroTCBiasSlope[ROLL] * sensors.gyroTemperature + cfg.gyroTCBiasIntercept[ROLL];
    sensors.gyroTCBias[PITCH] = cfg.gyroTCBiasSlope[PITCH] * sensors.gyroTemperature + cfg.gyroTCBiasIntercept[PITCH];
    sensors.gyroTCBias[YAW] = cfg.gyroTCBiasSlope[YAW] * sensors.gyroTemperature + cfg.gyroTCBiasIntercept[YAW];
}

///////////////////////////////////////////////////////////////////////////////
// Gyro Temperature Calibration
///////////////////////////////////////////////////////////////////////////////

void gyroTempCalibration(void)
{
    uint16_t i;
    uint16_t gyroSampleRate = 1000;
    uint16_t numberOfGyroSamples = 2000;

    float gyroBias1[3] = { 0.0f, 0.0f, 0.0f };
    float gyroTemperature1 = 0.0f;

    float gyroBias2[3] = { 0.0f, 00.f, 0.0f };
    float gyroTemperature2 = 0.0f;

    uartPrint("\nGyro Temperature Calibration:\n");

    ///////////////////////////////////
    // Get samples at temperature1
    ///////////////////////////////////
    uartPrint("\nBegin 1st Gyro Measurements...\n");
    for (i = 0; i < numberOfGyroSamples; i++) {
        readGyro();
        readGyroTemp();
        gyroBias1[ROLL] += rawGyro[ROLL];
        gyroBias1[PITCH] += rawGyro[PITCH];
        gyroBias1[YAW] += rawGyro[YAW];
        gyroTemperature1 += ((float) rawGyroTemperature + 13200.0f) / 280.0f + 35.0f;
        delayMicroseconds(gyroSampleRate);
    }

    gyroBias1[ROLL] /= (float) numberOfGyroSamples;
    gyroBias1[PITCH] /= (float) numberOfGyroSamples;
    gyroBias1[YAW] /= (float) numberOfGyroSamples;
    gyroTemperature1 /= (float) numberOfGyroSamples;

    uartPrint("\nGyro Temperature Reading: ");
    printf_min("%f\n\n", gyroTemperature1);
    uartPrint("\n\nEnd 1st Gyro Measurements\n");

    ///////////////////////////////////
    // Time delay for temperature
    // Stabilizaiton
    ///////////////////////////////////

    uartPrint("\nWaiting for 15 minutes for gyro temp to rise...\n");

    // Delay for 15 minutes
    for(i = 0; i < 450; ++i) {
        delay(2000);
        LED0_TOGGLE();
        readGyroTemp();
        printf_min("Temp: %f\n", ((float) rawGyroTemperature + 13200.0f) / 280.0f + 35.0f);
    }
    
    ///////////////////////////////////
    // Get samples at temperature2
    ///////////////////////////////////
    uartPrint("\nBegin 2nd Gyro Measurements...\n");
    for (i = 0; i < numberOfGyroSamples; i++) {
        readGyro();
        gyroBias2[ROLL] += rawGyro[ROLL];
        gyroBias2[PITCH] += rawGyro[PITCH];
        gyroBias2[YAW] += rawGyro[YAW];
        gyroTemperature2 += ((float) rawGyroTemperature + 13200.0f) / 280.0f + 35.0f;
        delayMicroseconds(gyroSampleRate);
    }

    gyroBias2[ROLL] /= (float) numberOfGyroSamples;
    gyroBias2[PITCH] /= (float) numberOfGyroSamples;
    gyroBias2[YAW] /= (float) numberOfGyroSamples;
    gyroTemperature2 /= (float) numberOfGyroSamples;

    uartPrint("\nGyro Temperature Reading: ");
    
    printf_min("%f\n\n", gyroTemperature2);
    uartPrint("\n\nEnd 2nd Gyro Measurements\n");

    cfg.gyroTCBiasSlope[ROLL] = (gyroBias2[ROLL] - gyroBias1[ROLL]) / (gyroTemperature2 - gyroTemperature1);
    cfg.gyroTCBiasSlope[PITCH] = (gyroBias2[PITCH] - gyroBias1[PITCH]) / (gyroTemperature2 - gyroTemperature1);
    cfg.gyroTCBiasSlope[YAW] = (gyroBias2[YAW] - gyroBias1[YAW]) / (gyroTemperature2 - gyroTemperature1);

    cfg.gyroTCBiasIntercept[ROLL] = gyroBias2[ROLL] - cfg.gyroTCBiasSlope[ROLL] * gyroTemperature2;
    cfg.gyroTCBiasIntercept[PITCH] = gyroBias2[PITCH] - cfg.gyroTCBiasSlope[PITCH] * gyroTemperature2;
    cfg.gyroTCBiasIntercept[YAW] = gyroBias2[YAW] - cfg.gyroTCBiasSlope[YAW] * gyroTemperature2;

    
    uartPrint("\nGyro TC Bias Slope[3]      = { ");
    printf_min("%f, %f, %f", cfg.gyroTCBiasSlope[ROLL], cfg.gyroTCBiasSlope[PITCH], cfg.gyroTCBiasSlope[YAW]);
    uartPrint(" }\n");

    uartPrint("\n Gyro TC Bias Intercept[3]  = { ");
    printf_min("%f, %f, %f", cfg.gyroTCBiasIntercept[ROLL], cfg.gyroTCBiasIntercept[PITCH], cfg.gyroTCBiasIntercept[YAW]);
    uartPrint(" }\n");

    uartPrint("\nGyro Temperature Calibration Complete.\n");

}

///////////////////////////////////////////////////////////////////////////////
// Compute Gyro Runtime Bias
///////////////////////////////////////////////////////////////////////////////

void computeGyroRTBias(void)
{
    uint8_t axis;
    uint16_t samples;
    float gyroSum[3] = { 0.0f, 0.0f, 0.0f };

    for (samples = 0; samples < 2000; samples++) {
        readGyro();
        readGyroTemp();

        computeGyroTCBias();

        gyroSum[ROLL] += rawGyro[ROLL] - sensors.gyroTCBias[ROLL];
        gyroSum[PITCH] += rawGyro[PITCH] - sensors.gyroTCBias[PITCH];
        gyroSum[YAW] += rawGyro[YAW] - sensors.gyroTCBias[YAW];
        
        if(!(samples % 250))
            LED0_TOGGLE();

        delay(1);
    }

    for (axis = ROLL; axis < 3; axis++) {
        sensors.gyroRTBias[axis] = (float) gyroSum[axis] / 2000.0f;

    }
}

void initGyro(void)
{
    gyro->init();
}