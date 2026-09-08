#ifndef SFE_LSM6DSV16X_H_
#define SFE_LSM6DSV16X_H_

#include <stdbool.h>
#include <stdint.h>

#include "lsm6dsv16x_reg.h"
#include "sfe_lsm_shim.h"

/*
 * LSM6DSV16X I2C addresses.
 *
 * MCUXpresso uses the 7-bit slave address.
 */
#define LSM6DSV16X_ADDRESS_LOW        0x6AU
#define LSM6DSV16X_ADDRESS_PRIMARY    0x6AU
#define LSM6DSV16X_ADDRESS_HIGH       0x6BU
#define LSM6DSV16X_ADDRESS_SECONDARY  0x6BU


/*
 * Interrupt pin selection.
 */
typedef enum
{
    LSM_PIN_ONE = 0x01,
    LSM_PIN_TWO = 0x02
} sfe_lsm_pin_t;


/*
 * Raw accelerometer / gyroscope data.
 */
typedef struct
{
    int16_t xData;
    int16_t yData;
    int16_t zData;
} sfe_lsm_raw_data_t;


/*
 * Converted accelerometer / gyroscope data.
 */
typedef struct
{
    float xData;
    float yData;
    float zData;
} sfe_lsm_data_t;


/*
 * Main LSM6DSV16X device object.
 *
 * This replaces the original C++ QwDevLSM6DSV16X class.
 */
typedef struct
{
    stmdev_ctx_t dev_ctx;

    sfe_lsm_platform_t platform;

    bool accelScaleSet;
    lsm6dsv16x_xl_full_scale_t fullScaleAccel;

    bool gyroScaleSet;
    lsm6dsv16x_gy_full_scale_t fullScaleGyro;

} sfe_lsm6dsv16x_t;


/* -------------------------------------------------------------------------- */
/* Initialization                                                             */
/* -------------------------------------------------------------------------- */

/**
 * @brief Initialize the LSM6DSV16X software device structure.
 *
 * @param dev         Pointer to device structure.
 * @param i2c_handle  Pointer to KL25Z I2C peripheral, normally I2C0.
 * @param address     7-bit LSM6DSV16X I2C address.
 *
 * @return true on success.
 */
bool SFE_LSM6DSV16X_Init(sfe_lsm6dsv16x_t *dev,
                         void *i2c_handle,
                         uint8_t address);


/**
 * @brief Check communication with the sensor.
 *
 * Reads WHO_AM_I and verifies the LSM6DSV16X device ID.
 */
bool SFE_LSM6DSV16X_IsConnected(sfe_lsm6dsv16x_t *dev);


/* -------------------------------------------------------------------------- */
/* Register access                                                            */
/* -------------------------------------------------------------------------- */

int32_t SFE_LSM6DSV16X_WriteRegisterRegion(sfe_lsm6dsv16x_t *dev,
                                           uint8_t reg,
                                           uint8_t *data,
                                           uint16_t length);

int32_t SFE_LSM6DSV16X_ReadRegisterRegion(sfe_lsm6dsv16x_t *dev,
                                          uint8_t reg,
                                          uint8_t *data,
                                          uint16_t length);


/* -------------------------------------------------------------------------- */
/* Device identification                                                      */
/* -------------------------------------------------------------------------- */

uint8_t SFE_LSM6DSV16X_GetUniqueId(sfe_lsm6dsv16x_t *dev);


/* -------------------------------------------------------------------------- */
/* Full-scale configuration                                                   */
/* -------------------------------------------------------------------------- */

bool SFE_LSM6DSV16X_SetAccelFullScale(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_xl_full_scale_t scale);

bool SFE_LSM6DSV16X_GetAccelFullScale(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_xl_full_scale_t *scale);

bool SFE_LSM6DSV16X_SetGyroFullScale(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_gy_full_scale_t scale);

bool SFE_LSM6DSV16X_GetGyroFullScale(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_gy_full_scale_t *scale);


/* -------------------------------------------------------------------------- */
/* Raw sensor data                                                            */
/* -------------------------------------------------------------------------- */

bool SFE_LSM6DSV16X_GetRawTemp(sfe_lsm6dsv16x_t *dev,
                               int16_t *temperature);

bool SFE_LSM6DSV16X_GetRawAccel(sfe_lsm6dsv16x_t *dev,
                                sfe_lsm_raw_data_t *accelData);

bool SFE_LSM6DSV16X_GetRawGyro(sfe_lsm6dsv16x_t *dev,
                               sfe_lsm_raw_data_t *gyroData);

bool SFE_LSM6DSV16X_GetRawQvar(sfe_lsm6dsv16x_t *dev,
                               int16_t *qvarData);


/* -------------------------------------------------------------------------- */
/* Converted sensor data                                                      */
/* -------------------------------------------------------------------------- */

bool SFE_LSM6DSV16X_GetAccel(sfe_lsm6dsv16x_t *dev,
                             sfe_lsm_data_t *accelData);

bool SFE_LSM6DSV16X_GetGyro(sfe_lsm6dsv16x_t *dev,
                            sfe_lsm_data_t *gyroData);


/* -------------------------------------------------------------------------- */
/* General configuration                                                      */
/* -------------------------------------------------------------------------- */

bool SFE_LSM6DSV16X_DeviceReset(sfe_lsm6dsv16x_t *dev);

bool SFE_LSM6DSV16X_GetDeviceReset(sfe_lsm6dsv16x_t *dev);

bool SFE_LSM6DSV16X_GetAutoIncrement(sfe_lsm6dsv16x_t *dev);

bool SFE_LSM6DSV16X_SetAccelMode(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_xl_mode_t mode);

bool SFE_LSM6DSV16X_SetGyroMode(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_gy_mode_t mode);

bool SFE_LSM6DSV16X_EnableAccelHpFilter(
        sfe_lsm6dsv16x_t *dev,
        bool enable);

bool SFE_LSM6DSV16X_EnableAccelLPS2(
        sfe_lsm6dsv16x_t *dev,
        bool enable);

bool SFE_LSM6DSV16X_EnableFastSetMode(
        sfe_lsm6dsv16x_t *dev,
        bool enable);

bool SFE_LSM6DSV16X_EnableGyroLP1Filter(
        sfe_lsm6dsv16x_t *dev,
        bool enable);

bool SFE_LSM6DSV16X_SetGyroLP1Bandwidth(
        sfe_lsm6dsv16x_t *dev,
        uint8_t val);

bool SFE_LSM6DSV16X_EnableAccelLP2Filter(
        sfe_lsm6dsv16x_t *dev,
        bool enable);

bool SFE_LSM6DSV16X_SetAccelLP2Bandwidth(
        sfe_lsm6dsv16x_t *dev,
        uint8_t val);

bool SFE_LSM6DSV16X_EnableBlockDataUpdate(
        sfe_lsm6dsv16x_t *dev,
        bool enable);

uint8_t SFE_LSM6DSV16X_GetBlockDataUpdate(
        sfe_lsm6dsv16x_t *dev);

bool SFE_LSM6DSV16X_SetAccelDataRate(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_data_rate_t rate);

bool SFE_LSM6DSV16X_SetGyroDataRate(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_data_rate_t rate);

bool SFE_LSM6DSV16X_EnableTimestamp(
        sfe_lsm6dsv16x_t *dev,
        bool enable);

bool SFE_LSM6DSV16X_EnableFilterSettling(
        sfe_lsm6dsv16x_t *dev,
        bool enable);


/* -------------------------------------------------------------------------- */
/* Interrupt configuration                                                    */
/* -------------------------------------------------------------------------- */

bool SFE_LSM6DSV16X_GetAllInterrupts(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_all_sources_t *source);

bool SFE_LSM6DSV16X_SetInt2DENActiveLow(
        sfe_lsm6dsv16x_t *dev,
        bool activeLow);

bool SFE_LSM6DSV16X_SetIntRoute(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_pin_int_route_t val,
        sfe_lsm_pin_t pin);

bool SFE_LSM6DSV16X_SetIntAccelDataReady(
        sfe_lsm6dsv16x_t *dev,
        sfe_lsm_pin_t pin,
        bool enable);

bool SFE_LSM6DSV16X_SetIntGyroDataReady(
        sfe_lsm6dsv16x_t *dev,
        sfe_lsm_pin_t pin,
        bool enable);

bool SFE_LSM6DSV16X_SetIntSingleTap(
        sfe_lsm6dsv16x_t *dev,
        sfe_lsm_pin_t pin,
        bool enable);

bool SFE_LSM6DSV16X_SetIntDoubleTap(
        sfe_lsm6dsv16x_t *dev,
        sfe_lsm_pin_t pin,
        bool enable);

bool SFE_LSM6DSV16X_SetIntWakeup(
        sfe_lsm6dsv16x_t *dev,
        sfe_lsm_pin_t pin,
        bool enable);

bool SFE_LSM6DSV16X_SetIntFreeFall(
        sfe_lsm6dsv16x_t *dev,
        sfe_lsm_pin_t pin,
        bool enable);

bool SFE_LSM6DSV16X_SetIntSleepChange(
        sfe_lsm6dsv16x_t *dev,
        sfe_lsm_pin_t pin,
        bool enable);

bool SFE_LSM6DSV16X_SetDataReadyMode(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_data_ready_mode_t mode);


/* -------------------------------------------------------------------------- */
/* Tap configuration                                                          */
/* -------------------------------------------------------------------------- */

bool SFE_LSM6DSV16X_EnableTapInterrupt(
        sfe_lsm6dsv16x_t *dev,
        bool enable);

bool SFE_LSM6DSV16X_SetTapMode(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_tap_mode_t mode);

bool SFE_LSM6DSV16X_GetTapMode(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_tap_mode_t *mode);

bool SFE_LSM6DSV16X_SetTapDirection(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_tap_detection_t directionDetect);

bool SFE_LSM6DSV16X_GetTapDirection(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_tap_detection_t *directionDetect);

bool SFE_LSM6DSV16X_SetTapThresholds(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_tap_thresholds_t thresholds);

bool SFE_LSM6DSV16X_GetTapThresholds(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_tap_thresholds_t *thresholds);

bool SFE_LSM6DSV16X_SetTapTimeWindows(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_tap_time_windows_t window);

bool SFE_LSM6DSV16X_GetTapTimeWindows(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_tap_time_windows_t *window);


/* -------------------------------------------------------------------------- */
/* QVAR                                                                       */
/* -------------------------------------------------------------------------- */

bool SFE_LSM6DSV16X_EnableAhQvar(
        sfe_lsm6dsv16x_t *dev,
        bool enable);

bool SFE_LSM6DSV16X_GetQvarMode(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_ah_qvar_mode_t *mode);

bool SFE_LSM6DSV16X_SetQvarImpedance(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_ah_qvar_zin_t val);


/* -------------------------------------------------------------------------- */
/* Sensor Hub                                                                 */
/* -------------------------------------------------------------------------- */

bool SFE_LSM6DSV16X_SetHubODR(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_sh_data_rate_t rate);

bool SFE_LSM6DSV16X_SetHubSensorRead(
        sfe_lsm6dsv16x_t *dev,
        uint8_t sensor,
        lsm6dsv16x_sh_cfg_read_t *settings);

bool SFE_LSM6DSV16X_SetHubSensorWrite(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_sh_cfg_write_t *settings);

bool SFE_LSM6DSV16X_SetNumberHubSensors(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_sh_slave_connected_t numSensors);

bool SFE_LSM6DSV16X_EnableAuxiliaryI2C(
        sfe_lsm6dsv16x_t *dev,
        bool enable);

bool SFE_LSM6DSV16X_ReadPeripheralSensor(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_emb_sh_read_t *shReg,
        uint8_t len);

bool SFE_LSM6DSV16X_GetExternalSensorNack(
        sfe_lsm6dsv16x_t *dev,
        uint8_t sensor);

bool SFE_LSM6DSV16X_EnableHubWriteOnceMode(
        sfe_lsm6dsv16x_t *dev,
        bool enable);

bool SFE_LSM6DSV16X_EnableHubPassThrough(
        sfe_lsm6dsv16x_t *dev,
        bool enable);

bool SFE_LSM6DSV16X_EnableHubPullUps(
        sfe_lsm6dsv16x_t *dev,
        bool enable);

bool SFE_LSM6DSV16X_ResetSensorHub(
        sfe_lsm6dsv16x_t *dev);


/* -------------------------------------------------------------------------- */
/* Self-test                                                                  */
/* -------------------------------------------------------------------------- */

bool SFE_LSM6DSV16X_SetAccelSelfTest(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_xl_self_test_t val);

bool SFE_LSM6DSV16X_SetGyroSelfTest(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_gy_self_test_t val);


/* -------------------------------------------------------------------------- */
/* FIFO                                                                       */
/* -------------------------------------------------------------------------- */

bool SFE_LSM6DSV16X_SetFifoWatermark(
        sfe_lsm6dsv16x_t *dev,
        uint8_t val);

bool SFE_LSM6DSV16X_SetFifoMode(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_fifo_mode_t mode);

bool SFE_LSM6DSV16X_SetAccelFifoBatchSet(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_fifo_xl_batch_t odr);

bool SFE_LSM6DSV16X_SetGyroFifoBatchSet(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_fifo_gy_batch_t odr);

bool SFE_LSM6DSV16X_SetFifoTimestampDec(
        sfe_lsm6dsv16x_t *dev,
        lsm6dsv16x_fifo_timestamp_batch_t decimation);


/* -------------------------------------------------------------------------- */
/* Status                                                                     */
/* -------------------------------------------------------------------------- */

bool SFE_LSM6DSV16X_CheckStatus(
        sfe_lsm6dsv16x_t *dev);

bool SFE_LSM6DSV16X_CheckAccelStatus(
        sfe_lsm6dsv16x_t *dev);

bool SFE_LSM6DSV16X_CheckGyroStatus(
        sfe_lsm6dsv16x_t *dev);

bool SFE_LSM6DSV16X_CheckTempStatus(
        sfe_lsm6dsv16x_t *dev);

bool SFE_LSM6DSV16X_CheckQvar(
        sfe_lsm6dsv16x_t *dev);


/* -------------------------------------------------------------------------- */
/* Conversion functions                                                       */
/* -------------------------------------------------------------------------- */

float SFE_LSM6DSV16X_Convert2gToMg(int16_t data);
float SFE_LSM6DSV16X_Convert4gToMg(int16_t data);
float SFE_LSM6DSV16X_Convert8gToMg(int16_t data);
float SFE_LSM6DSV16X_Convert16gToMg(int16_t data);

float SFE_LSM6DSV16X_Convert125dpsToMdps(int16_t data);
float SFE_LSM6DSV16X_Convert250dpsToMdps(int16_t data);
float SFE_LSM6DSV16X_Convert500dpsToMdps(int16_t data);
float SFE_LSM6DSV16X_Convert1000dpsToMdps(int16_t data);
float SFE_LSM6DSV16X_Convert2000dpsToMdps(int16_t data);
float SFE_LSM6DSV16X_Convert4000dpsToMdps(int16_t data);

float SFE_LSM6DSV16X_ConvertToCelsius(int16_t data);


#endif /* SFE_LSM6DSV16X_H_ */
