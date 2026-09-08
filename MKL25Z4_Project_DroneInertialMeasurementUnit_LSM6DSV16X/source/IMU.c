//#include <stdio.h>
//
//#include "board.h"
//#include "pin_mux.h"
//#include "clock_config.h"
//#include "fsl_debug_console.h"
//#include "fsl_i2c.h"
//
//#include "sfe_lsm6dsv16x.h"
//
//#define LSM6DSV16X_I2C_BASE        I2C0
//#define LSM6DSV16X_I2C_BAUDRATE    100000U
//
//
//static void delay_ms(volatile uint32_t ms)
//{
//    volatile uint32_t count;
//
//    while (ms--)
//    {
//        count = 6000U;
//
//        while (count--)
//        {
//            __asm volatile ("nop");
//        }
//    }
//}
//
//
//
//int main(void)
//{
//    sfe_lsm6dsv16x_t imu;
//    sfe_lsm_data_t accelData;
//    sfe_lsm_data_t gyroData;
//
//    i2c_master_config_t i2cConfig;
//
//    bool status;
//
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitDebugConsole();
//
//    PRINTF("\r\n");
//    PRINTF("LSM6DSV16X KL25Z Test\r\n");
//    PRINTF("----------------------\r\n");
//
//    /*
//     * Initialize I2C0.
//     */
//    I2C_MasterGetDefaultConfig(&i2cConfig);
//
//    i2cConfig.baudRate_Bps = LSM6DSV16X_I2C_BAUDRATE;
//
//    I2C_MasterInit(LSM6DSV16X_I2C_BASE,
//                   &i2cConfig,
//                   CLOCK_GetFreq(I2C0_CLK_SRC));
//
//    /*
//     * Initialize the IMU software driver.
//     *
//     * Try 0x6B first.
//     */
//    status = SFE_LSM6DSV16X_Init(&imu,
//                                 (void *)LSM6DSV16X_I2C_BASE,
//                                 LSM6DSV16X_ADDRESS_HIGH);
//
//    if (!status)
//    {
//        PRINTF("ERROR: LSM6DSV16X not detected at 0x6B\r\n");
//
//        while (1)
//        {
//        }
//    }
//
//    PRINTF("LSM6DSV16X detected successfully.\r\n");
//
//    /*
//     * Configure accelerometer.
//     */
//    if (!SFE_LSM6DSV16X_SetAccelFullScale(&imu,
//                                          LSM6DSV16X_2g))
//    {
//        PRINTF("ERROR: Failed to set accelerometer scale\r\n");
//    }
//
//    if (!SFE_LSM6DSV16X_SetAccelDataRate(&imu,
//                                         LSM6DSV16X_ODR_AT_120Hz))
//    {
//        PRINTF("ERROR: Failed to set accelerometer ODR\r\n");
//    }
//
//    /*
//     * Configure gyroscope.
//     */
//    if (!SFE_LSM6DSV16X_SetGyroFullScale(&imu,
//                                         LSM6DSV16X_250dps))
//    {
//        PRINTF("ERROR: Failed to set gyroscope scale\r\n");
//    }
//
//    if (!SFE_LSM6DSV16X_SetGyroDataRate(&imu,
//                                        LSM6DSV16X_ODR_AT_120Hz))
//    {
//        PRINTF("ERROR: Failed to set gyroscope ODR\r\n");
//    }
//
//    /*
//     * Block Data Update is useful for multi-byte sensor reads.
//     */
//    SFE_LSM6DSV16X_EnableBlockDataUpdate(&imu, true);
//
//    PRINTF("IMU configured.\r\n\r\n");
//
//    while (1)
//    {
//        if (SFE_LSM6DSV16X_GetAccel(&imu, &accelData) &&
//            SFE_LSM6DSV16X_GetGyro(&imu, &gyroData))
//        {
//            PRINTF("ACCEL [mg]  X=%d  Y=%d  Z=%d\r\n",
//                   (int)accelData.xData,
//                   (int)accelData.yData,
//                   (int)accelData.zData);
//
//            PRINTF("GYRO [mdps] X=%d  Y=%d  Z=%d\r\n",
//                   (int)gyroData.xData,
//                   (int)gyroData.yData,
//                   (int)gyroData.zData);
//
//            PRINTF("\r\n");
//        }
//        else
//        {
//            PRINTF("ERROR: Failed to read IMU\r\n");
//        }
//
//        //SDK_DelayAtLeastUs(100000U, SystemCoreClock);
//        delay_ms(100U);
//}
//}












//
//
//
//
//
//#include <stdbool.h>
//#include <stdint.h>
//
//#include "board.h"
//#include "pin_mux.h"
//#include "clock_config.h"
//
//#include "fsl_debug_console.h"
//#include "fsl_i2c.h"
//
//#include "sfe_lsm6dsv16x.h"
//
//
//#define LSM6DSV16X_I2C_BASE       I2C0
//#define LSM6DSV16X_I2C_BAUDRATE   100000U
////#define GYRO_CAL_SAMPLES           1000U
//
//static void delay_ms(volatile uint32_t ms)
//{
//    volatile uint32_t count;
//
//    while (ms--)
//    {
//        count = 6000U;
//
//        while (count--)
//        {
//            __asm volatile ("nop");
//        }
//    }
//}
//
//
//int main(void)
//{
//    sfe_lsm6dsv16x_t imu;
//
//    sfe_lsm_data_t accelData;
//    sfe_lsm_data_t gyroData;
//
//    i2c_master_config_t i2cConfig;
//
//    bool status;
//    //float gyroBiasX = 0.0f;
//    //float gyroBiasY = 0.0f;
//    //float gyroBiasZ = 0.0f;
//
//    /* ---------------------------------------------------------------------- */
//    /* KL25Z board initialization                                             */
//    /* ---------------------------------------------------------------------- */
//
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitDebugConsole();
//
//
//    PRINTF("\r\n");
//    PRINTF("LSM6DSV16X KL25Z Test\r\n");
//    PRINTF("----------------------\r\n");
//
//
//    /* ---------------------------------------------------------------------- */
//    /* Initialize I2C0                                                        */
//    /* ---------------------------------------------------------------------- */
//
//    PRINTF("1: Getting I2C configuration...\r\n");
//
//    I2C_MasterGetDefaultConfig(&i2cConfig);
//
//    PRINTF("2: I2C configuration obtained.\r\n");
//
//
//    i2cConfig.baudRate_Bps = LSM6DSV16X_I2C_BAUDRATE;
//
//
//    PRINTF("3: Initializing I2C0...\r\n");
//
//    I2C_MasterInit(LSM6DSV16X_I2C_BASE,
//                   &i2cConfig,
//                   CLOCK_GetFreq(I2C0_CLK_SRC));
//
//    PRINTF("4: I2C0 initialized.\r\n");
//
//
//    /* ---------------------------------------------------------------------- */
//    /* Detect LSM6DSV16X                                                      */
//    /* ---------------------------------------------------------------------- */
//
//    PRINTF("5: Attempting IMU communication at address 0x6A...\r\n");
//
//
//    status = SFE_LSM6DSV16X_Init(&imu,
//                                 (void *)LSM6DSV16X_I2C_BASE,
//                                 LSM6DSV16X_ADDRESS_HIGH);
//
//
//    PRINTF("6: SFE_LSM6DSV16X_Init returned.\r\n");
//
//
//    if (!status)
//    {
//        PRINTF("ERROR: LSM6DSV16X not detected at 0x6B.\r\n");
//
//        while (1)
//        {
//        }
//    }
//
//
//    PRINTF("LSM6DSV16X detected successfully.\r\n");
//
//
//    /* ---------------------------------------------------------------------- */
//    /* Configure accelerometer                                                */
//    /* ---------------------------------------------------------------------- */
//
//    PRINTF("7: Configuring accelerometer...\r\n");
//
//
//    if (!SFE_LSM6DSV16X_SetAccelFullScale(&imu,
//                                          LSM6DSV16X_2g))
//    {
//        PRINTF("ERROR: Failed to set accelerometer full scale.\r\n");
//
//        while (1)
//        {
//        }
//    }
//
//
//    if (!SFE_LSM6DSV16X_SetAccelDataRate(&imu,
//                                         LSM6DSV16X_ODR_AT_120Hz))
//    {
//        PRINTF("ERROR: Failed to set accelerometer data rate.\r\n");
//
//        while (1)
//        {
//        }
//    }
//
//
//    PRINTF("8: Accelerometer configured.\r\n");
//
//
//    /* ---------------------------------------------------------------------- */
//    /* Configure gyroscope                                                    */
//    /* ---------------------------------------------------------------------- */
//
//    PRINTF("9: Configuring gyroscope...\r\n");
//
//
//    if (!SFE_LSM6DSV16X_SetGyroFullScale(&imu,
//                                         LSM6DSV16X_250dps))
//    {
//        PRINTF("ERROR: Failed to set gyroscope full scale.\r\n");
//
//        while (1)
//        {
//        }
//    }
//
//
//    if (!SFE_LSM6DSV16X_SetGyroDataRate(&imu,
//                                        LSM6DSV16X_ODR_AT_120Hz))
//    {
//        PRINTF("ERROR: Failed to set gyroscope data rate.\r\n");
//
//        while (1)
//        {
//        }
//    }
//
//
//    PRINTF("10: Gyroscope configured.\r\n");
//
//
//    /* ---------------------------------------------------------------------- */
//    /* Enable Block Data Update                                               */
//    /* ---------------------------------------------------------------------- */
//
//    if (!SFE_LSM6DSV16X_EnableBlockDataUpdate(&imu, true))
//    {
//        PRINTF("ERROR: Failed to enable Block Data Update.\r\n");
//
//        while (1)
//        {
//        }
//    }
//
//
//    PRINTF("11: Block Data Update enabled.\r\n");
//    PRINTF("\r\n");
//    PRINTF("IMU configuration complete.\r\n");
//    PRINTF("Starting sensor output...\r\n");
//    PRINTF("\r\n");
//
//
//    /* ---------------------------------------------------------------------- */
//    /* Main sensor read loop                                                  */
//    /* ---------------------------------------------------------------------- */
//
//    while (1)
//    {
//        bool accelSuccess;
//        bool gyroSuccess;
//
//
//        accelSuccess = SFE_LSM6DSV16X_GetAccel(&imu,
//                                               &accelData);
//
//        gyroSuccess = SFE_LSM6DSV16X_GetGyro(&imu,
//                                             &gyroData);
//
//
//        if (accelSuccess && gyroSuccess)
//        {
//            /*
//             * Accelerometer values are returned in mg.
//             *
//             * Gyroscope values are returned in mdps.
//             *
//             * Float printing is currently disabled in the MCUXpresso
//             * project, so cast the values to integers for terminal output.
//             */
//
//            PRINTF("ACCEL [mg]   X=%d   Y=%d   Z=%d\r\n",
//                   (int)accelData.xData,
//                   (int)accelData.yData,
//                   (int)accelData.zData);
//
//
//            PRINTF("GYRO [mdps]  X=%d   Y=%d   Z=%d\r\n",
//                   (int)gyroData.xData,
//                   (int)gyroData.yData,
//                   (int)gyroData.zData);
//
//
//            PRINTF("\r\n");
//        }
//        else
//        {
//            PRINTF("ERROR: Failed to read IMU sensor data.\r\n");
//        }
//
//
//        delay_ms(1000U);
//    }
//}

























#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "fsl_debug_console.h"
#include "fsl_i2c.h"

#include "sfe_lsm6dsv16x.h"


#define LSM6DSV16X_I2C_BASE       I2C0
#define LSM6DSV16X_I2C_BAUDRATE   100000U

#define GYRO_CAL_SAMPLES           1000U


static void delay_ms(volatile uint32_t ms)
{
    volatile uint32_t count;

    while (ms--)
    {
        count = 6000U;

        while (count--)
        {
            __asm volatile ("nop");
        }
    }
}


int main(void)
{
    sfe_lsm6dsv16x_t imu;

    sfe_lsm_data_t accelData;
    sfe_lsm_data_t gyroData;

    i2c_master_config_t i2cConfig;

    bool status;

    /*
     * Gyroscope zero-rate bias.
     *
     * These values will be calculated automatically during startup
     * while the IMU is sitting completely still.
     *
     * Units: mdps
     */
    float gyroBiasX = 0.0f;
    float gyroBiasY = 0.0f;
    float gyroBiasZ = 0.0f;


    /* ---------------------------------------------------------------------- */
    /* KL25Z board initialization                                             */
    /* ---------------------------------------------------------------------- */

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();


    PRINTF("\r\n");
    PRINTF("LSM6DSV16X KL25Z Test\r\n");
    PRINTF("----------------------\r\n");


    /* ---------------------------------------------------------------------- */
    /* Initialize I2C0                                                        */
    /* ---------------------------------------------------------------------- */

    PRINTF("1: Getting I2C configuration...\r\n");

    I2C_MasterGetDefaultConfig(&i2cConfig);

    PRINTF("2: I2C configuration obtained.\r\n");


    i2cConfig.baudRate_Bps = LSM6DSV16X_I2C_BAUDRATE;


    PRINTF("3: Initializing I2C0...\r\n");

    I2C_MasterInit(LSM6DSV16X_I2C_BASE,
                   &i2cConfig,
                   CLOCK_GetFreq(I2C0_CLK_SRC));

    PRINTF("4: I2C0 initialized.\r\n");


    /* ---------------------------------------------------------------------- */
    /* Detect LSM6DSV16X                                                      */
    /* ---------------------------------------------------------------------- */

    PRINTF("5: Attempting IMU communication at address 0x6B...\r\n");


    status = SFE_LSM6DSV16X_Init(&imu,
                                 (void *)LSM6DSV16X_I2C_BASE,
                                 LSM6DSV16X_ADDRESS_HIGH);


    PRINTF("6: SFE_LSM6DSV16X_Init returned.\r\n");


    if (!status)
    {
        PRINTF("ERROR: LSM6DSV16X not detected at 0x6B.\r\n");

        while (1)
        {
        }
    }


    PRINTF("LSM6DSV16X detected successfully.\r\n");


    /* ---------------------------------------------------------------------- */
    /* Configure accelerometer                                                */
    /* ---------------------------------------------------------------------- */

    PRINTF("7: Configuring accelerometer...\r\n");


    if (!SFE_LSM6DSV16X_SetAccelFullScale(&imu,
                                          LSM6DSV16X_2g))
    {
        PRINTF("ERROR: Failed to set accelerometer full scale.\r\n");

        while (1)
        {
        }
    }


    if (!SFE_LSM6DSV16X_SetAccelDataRate(&imu,
                                         LSM6DSV16X_ODR_AT_120Hz))
    {
        PRINTF("ERROR: Failed to set accelerometer data rate.\r\n");

        while (1)
        {
        }
    }


    PRINTF("8: Accelerometer configured.\r\n");


    /* ---------------------------------------------------------------------- */
    /* Configure gyroscope                                                    */
    /* ---------------------------------------------------------------------- */

    PRINTF("9: Configuring gyroscope...\r\n");


    if (!SFE_LSM6DSV16X_SetGyroFullScale(&imu,
                                         LSM6DSV16X_250dps))
    {
        PRINTF("ERROR: Failed to set gyroscope full scale.\r\n");

        while (1)
        {
        }
    }


    if (!SFE_LSM6DSV16X_SetGyroDataRate(&imu,
                                        LSM6DSV16X_ODR_AT_120Hz))
    {
        PRINTF("ERROR: Failed to set gyroscope data rate.\r\n");

        while (1)
        {
        }
    }


    PRINTF("10: Gyroscope configured.\r\n");


    /* ---------------------------------------------------------------------- */
    /* Enable Block Data Update                                               */
    /* ---------------------------------------------------------------------- */

    if (!SFE_LSM6DSV16X_EnableBlockDataUpdate(&imu, true))
    {
        PRINTF("ERROR: Failed to enable Block Data Update.\r\n");

        while (1)
        {
        }
    }


    PRINTF("11: Block Data Update enabled.\r\n");


    /* ---------------------------------------------------------------------- */
    /* Allow IMU to stabilize                                                 */
    /* ---------------------------------------------------------------------- */

    PRINTF("\r\n");
    PRINTF("IMU configuration complete.\r\n");
    PRINTF("\r\n");

    PRINTF("IMPORTANT: Keep the IMU completely still.\r\n");
    PRINTF("Gyroscope calibration will begin shortly...\r\n");

    /*
     * Give the sensor some time to settle before collecting
     * calibration samples.
     */
    delay_ms(2000U);


    /* ---------------------------------------------------------------------- */
    /* Gyroscope zero-rate bias calibration                                   */
    /* ---------------------------------------------------------------------- */

    {
        float gyroSumX = 0.0f;
        float gyroSumY = 0.0f;
        float gyroSumZ = 0.0f;

        uint32_t goodSamples = 0U;


        PRINTF("Starting gyroscope calibration...\r\n");


        while (goodSamples < GYRO_CAL_SAMPLES)
        {
            if (SFE_LSM6DSV16X_GetGyro(&imu, &gyroData))
            {
                gyroSumX += gyroData.xData;
                gyroSumY += gyroData.yData;
                gyroSumZ += gyroData.zData;

                goodSamples++;
            }

            /*
             * Gyroscope is configured for 120 Hz.
             *
             * A new measurement occurs approximately every 8.33 ms.
             * Waiting 10 ms prevents us from repeatedly sampling
             * significantly faster than the configured ODR.
             */
            delay_ms(10U);
        }


        /*
         * Calculate the average stationary reading for each axis.
         *
         * These become our zero-rate gyro biases.
         */
        gyroBiasX = gyroSumX / (float)GYRO_CAL_SAMPLES;
        gyroBiasY = gyroSumY / (float)GYRO_CAL_SAMPLES;
        gyroBiasZ = gyroSumZ / (float)GYRO_CAL_SAMPLES;
    }


    PRINTF("Gyroscope calibration complete.\r\n");


    /*
     * Float printing is disabled in the MCUXpresso project,
     * so print the calculated biases as integers.
     */
    PRINTF("Calculated gyro bias [mdps]:\r\n");

    PRINTF("X = %d\r\n", (int)gyroBiasX);
    PRINTF("Y = %d\r\n", (int)gyroBiasY);
    PRINTF("Z = %d\r\n", (int)gyroBiasZ);


    PRINTF("\r\n");
    PRINTF("Starting corrected sensor output...\r\n");
    PRINTF("\r\n");


    /* ---------------------------------------------------------------------- */
    /* Main sensor read loop                                                  */
    /* ---------------------------------------------------------------------- */

    while (1)
    {
        bool accelSuccess;
        bool gyroSuccess;

        float correctedGyroX;
        float correctedGyroY;
        float correctedGyroZ;


        accelSuccess = SFE_LSM6DSV16X_GetAccel(&imu,
                                               &accelData);

        gyroSuccess = SFE_LSM6DSV16X_GetGyro(&imu,
                                             &gyroData);


        if (accelSuccess && gyroSuccess)
        {
            /*
             * Subtract the startup zero-rate bias from each
             * gyroscope measurement.
             */
            correctedGyroX = gyroData.xData - gyroBiasX;
            correctedGyroY = gyroData.yData - gyroBiasY;
            correctedGyroZ = gyroData.zData - gyroBiasZ;


            /* -------------------------------------------------------------- */
            /* Accelerometer                                                  */
            /* -------------------------------------------------------------- */

            PRINTF("ACCEL [mg]   X=%d   Y=%d   Z=%d\r\n",
                   (int)accelData.xData,
                   (int)accelData.yData,
                   (int)accelData.zData);


            /* -------------------------------------------------------------- */
            /* Raw gyroscope                                                  */
            /* -------------------------------------------------------------- */

            PRINTF("GYRO RAW [mdps]   X=%d   Y=%d   Z=%d\r\n",
                   (int)gyroData.xData,
                   (int)gyroData.yData,
                   (int)gyroData.zData);


            /* -------------------------------------------------------------- */
            /* Bias-corrected gyroscope                                       */
            /* -------------------------------------------------------------- */

            PRINTF("GYRO CORR [mdps]  X=%d   Y=%d   Z=%d\r\n",
                   (int)correctedGyroX,
                   (int)correctedGyroY,
                   (int)correctedGyroZ);


            PRINTF("\r\n");
        }
        else
        {
            PRINTF("ERROR: Failed to read IMU sensor data.\r\n");
        }


        /*
         * Keep the slow 1-second output for now because we're testing
         * calibration, not running the actual flight-control loop yet.
         */
        delay_ms(1000U);
    }
}












