/*
 * compass.c
 *
 * Pure-C MMC5983MA driver for FRDM-KL25Z / MCUXpresso SDK.
 *
 * Based on the Blue Robotics mmc5983-python driver, adapted for:
 *   - MCUXpresso SDK fsl_i2c.h
 *   - blocking I2C transfers
 *   - single-shot magnetic measurements
 *   - Meas_M_Done polling
 *   - explicit SET / RESET bridge-offset calibration
 *
 * Hardware:
 *   I2C0
 *   PTC8 = I2C0_SCL
 *   PTC9 = I2C0_SDA
 *
 * IMPORTANT:
 *   This file implements SENSOR bridge/null-offset compensation.
 *   It does not yet perform environmental hard-iron or soft-iron calibration.
 */

#include "compass.h"

#include "fsl_clock.h"
#include "fsl_common.h"

/* -------------------------------------------------------------------------- */
/* Local delay                                                                */
/* -------------------------------------------------------------------------- */

static void COMPASS_DelayUs(uint32_t delay_us)
{
    uint32_t core_clock_hz;
    uint64_t ticks_remaining;

    if (delay_us == 0U)
    {
        return;
    }

    core_clock_hz = CLOCK_GetFreq(kCLOCK_CoreSysClk);

    ticks_remaining =
        ((uint64_t)core_clock_hz * (uint64_t)delay_us) / 1000000ULL;

    while (ticks_remaining != 0ULL)
    {
        uint32_t chunk;

        if (ticks_remaining > 0x01000000ULL)
        {
            chunk = 0x01000000UL;
        }
        else
        {
            chunk = (uint32_t)ticks_remaining;
        }

        if (chunk == 0U)
        {
            chunk = 1U;
        }

        SysTick->CTRL = 0U;
        SysTick->LOAD = chunk - 1U;
        SysTick->VAL  = 0U;

        SysTick->CTRL =
            SysTick_CTRL_CLKSOURCE_Msk |
            SysTick_CTRL_ENABLE_Msk;

        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U)
        {
        }

        SysTick->CTRL = 0U;

        if (ticks_remaining >= (uint64_t)chunk)
        {
            ticks_remaining -= (uint64_t)chunk;
        }
        else
        {
            ticks_remaining = 0ULL;
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Low-level I2C helpers                                                      */
/* -------------------------------------------------------------------------- */

static bool COMPASS_WriteRegister(compass_t *sensor,
                                  uint8_t reg,
                                  uint8_t value)
{
    i2c_master_transfer_t transfer;

    if ((sensor == NULL) || (sensor->i2c_base == NULL))
    {
        return false;
    }

    transfer.flags          = kI2C_TransferDefaultFlag;
    transfer.slaveAddress   = COMPASS_I2C_ADDRESS;
    transfer.direction      = kI2C_Write;
    transfer.subaddress     = reg;
    transfer.subaddressSize = 1U;
    transfer.data           = &value;
    transfer.dataSize       = 1U;

    sensor->last_i2c_status =
        I2C_MasterTransferBlocking(sensor->i2c_base, &transfer);

    return (sensor->last_i2c_status == kStatus_Success);
}

static bool COMPASS_ReadRegisters(compass_t *sensor,
                                  uint8_t start_reg,
                                  uint8_t *data,
                                  size_t data_size)
{
    i2c_master_transfer_t transfer;

    if ((sensor == NULL) ||
        (sensor->i2c_base == NULL) ||
        (data == NULL) ||
        (data_size == 0U))
    {
        return false;
    }

    transfer.flags          = kI2C_TransferDefaultFlag;
    transfer.slaveAddress   = COMPASS_I2C_ADDRESS;
    transfer.direction      = kI2C_Read;
    transfer.subaddress     = start_reg;
    transfer.subaddressSize = 1U;
    transfer.data           = data;
    transfer.dataSize       = data_size;

    sensor->last_i2c_status =
        I2C_MasterTransferBlocking(sensor->i2c_base, &transfer);

    return (sensor->last_i2c_status == kStatus_Success);
}

static bool COMPASS_ReadRegister(compass_t *sensor,
                                 uint8_t reg,
                                 uint8_t *value)
{
    return COMPASS_ReadRegisters(sensor, reg, value, 1U);
}

/* -------------------------------------------------------------------------- */
/* Raw-data conversion                                                        */
/* -------------------------------------------------------------------------- */

static int32_t COMPASS_Unsigned18ToSigned(uint32_t raw18)
{
    return (int32_t)raw18 - (int32_t)COMPASS_RAW_ZERO;
}

static void COMPASS_DecodeMeasurement(const uint8_t raw[8],
                                      compass_t *sensor,
                                      compass_data_t *data)
{
    uint32_t x_u18;
    uint32_t y_u18;
    uint32_t z_u18;

    /*
     * MMC5983MA 18-bit output packing:
     *
     * X = Xout0[7:0] : Xout1[7:0] : XYZout2[7:6]
     * Y = Yout0[7:0] : Yout1[7:0] : XYZout2[5:4]
     * Z = Zout0[7:0] : Zout1[7:0] : XYZout2[3:2]
     */
    x_u18 =
        ((uint32_t)raw[0] << 10) |
        ((uint32_t)raw[1] << 2)  |
        (((uint32_t)raw[6] & 0xC0U) >> 6);

    y_u18 =
        ((uint32_t)raw[2] << 10) |
        ((uint32_t)raw[3] << 2)  |
        (((uint32_t)raw[6] & 0x30U) >> 4);

    z_u18 =
        ((uint32_t)raw[4] << 10) |
        ((uint32_t)raw[5] << 2)  |
        (((uint32_t)raw[6] & 0x0CU) >> 2);

    data->x_raw = COMPASS_Unsigned18ToSigned(x_u18);
    data->y_raw = COMPASS_Unsigned18ToSigned(y_u18);
    data->z_raw = COMPASS_Unsigned18ToSigned(z_u18);

    data->x_gauss_raw =
        (float)data->x_raw / COMPASS_COUNTS_PER_GAUSS;

    data->y_gauss_raw =
        (float)data->y_raw / COMPASS_COUNTS_PER_GAUSS;

    data->z_gauss_raw =
        (float)data->z_raw / COMPASS_COUNTS_PER_GAUSS;

    if (sensor->bridge_offset_valid)
    {
        data->x_gauss =
            data->x_gauss_raw - sensor->bridge_offset_gauss[0];

        data->y_gauss =
            data->y_gauss_raw - sensor->bridge_offset_gauss[1];

        data->z_gauss =
            data->z_gauss_raw - sensor->bridge_offset_gauss[2];
    }
    else
    {
        data->x_gauss = data->x_gauss_raw;
        data->y_gauss = data->y_gauss_raw;
        data->z_gauss = data->z_gauss_raw;
    }

    data->temperature_raw = raw[7];

    data->temperature_c =
        ((float)data->temperature_raw * 200.0f / 256.0f) - 75.0f;
}

/* -------------------------------------------------------------------------- */
/* Public functions                                                           */
/* -------------------------------------------------------------------------- */

void COMPASS_Init(compass_t *sensor, I2C_Type *i2c_base)
{
    if (sensor == NULL)
    {
        return;
    }

    sensor->i2c_base = i2c_base;
    sensor->last_i2c_status = kStatus_Success;

    sensor->bridge_offset_gauss[0] = 0.0f;
    sensor->bridge_offset_gauss[1] = 0.0f;
    sensor->bridge_offset_gauss[2] = 0.0f;

    sensor->product_id = 0U;
    sensor->bridge_offset_valid = false;
}

bool COMPASS_Begin(compass_t *sensor)
{
    uint8_t product_id;

    if ((sensor == NULL) || (sensor->i2c_base == NULL))
    {
        return false;
    }

    COMPASS_ClearBridgeOffset(sensor);

    if (!COMPASS_SoftwareReset(sensor))
    {
        return false;
    }

    if (!COMPASS_ReadProductId(sensor, &product_id))
    {
        return false;
    }

    sensor->product_id = product_id;

    if (product_id != COMPASS_PRODUCT_ID_EXPECTED)
    {
        return false;
    }

    /*
     * Keep the sensor out of continuous-measurement mode.
     * We will explicitly trigger each measurement.
     */
    if (!COMPASS_WriteRegister(sensor, COMPASS_REG_CONTROL2, 0x00U))
    {
        return false;
    }

    /*
     * Start with the 100 Hz bandwidth setting.
     * This gives an 8 ms conversion time; our measurement routine polls
     * Meas_M_Done instead of assuming a fixed delay.
     */
    if (!COMPASS_SetBandwidth(sensor, kCompassBandwidth100Hz))
    {
        return false;
    }

    /*
     * Condition the AMR bridge into SET polarity so normal single-shot
     * readings have the +H polarity.
     */
    if (!COMPASS_Set(sensor))
    {
        return false;
    }

    return true;
}

bool COMPASS_ReadProductId(compass_t *sensor, uint8_t *product_id)
{
    if (product_id == NULL)
    {
        return false;
    }

    return COMPASS_ReadRegister(sensor,
                                COMPASS_REG_PRODUCT_ID,
                                product_id);
}

bool COMPASS_SoftwareReset(compass_t *sensor)
{
    if (!COMPASS_WriteRegister(sensor,
                               COMPASS_REG_CONTROL1,
                               COMPASS_CONTROL1_SW_RESET))
    {
        return false;
    }

    COMPASS_DelayUs(COMPASS_SOFTWARE_RESET_DELAY_US);

    return true;
}

bool COMPASS_SetBandwidth(compass_t *sensor,
                          compass_bandwidth_t bandwidth)
{
    uint8_t value;

    if ((uint32_t)bandwidth > (uint32_t)kCompassBandwidth800Hz)
    {
        return false;
    }

    value = ((uint8_t)bandwidth & COMPASS_CONTROL1_BW_MASK);

    return COMPASS_WriteRegister(sensor,
                                 COMPASS_REG_CONTROL1,
                                 value);
}

bool COMPASS_Set(compass_t *sensor)
{
    if (!COMPASS_WriteRegister(sensor,
                               COMPASS_REG_CONTROL0,
                               COMPASS_CONTROL0_SET))
    {
        return false;
    }

    COMPASS_DelayUs(COMPASS_SET_RESET_DELAY_US);

    return true;
}

bool COMPASS_Reset(compass_t *sensor)
{
    if (!COMPASS_WriteRegister(sensor,
                               COMPASS_REG_CONTROL0,
                               COMPASS_CONTROL0_RESET))
    {
        return false;
    }

    COMPASS_DelayUs(COMPASS_SET_RESET_DELAY_US);

    return true;
}

bool COMPASS_WaitForMagneticMeasurement(compass_t *sensor,
                                        uint32_t timeout_us)
{
    uint32_t elapsed_us = 0U;
    uint8_t status;

    if (sensor == NULL)
    {
        return false;
    }

    while (elapsed_us <= timeout_us)
    {
        if (!COMPASS_ReadRegister(sensor,
                                  COMPASS_REG_STATUS,
                                  &status))
        {
            return false;
        }

        if ((status & COMPASS_STATUS_MEAS_M_DONE) != 0U)
        {
            return true;
        }

        COMPASS_DelayUs(COMPASS_STATUS_POLL_INTERVAL_US);

        elapsed_us += COMPASS_STATUS_POLL_INTERVAL_US;
    }

    return false;
}

bool COMPASS_ReadData(compass_t *sensor, compass_data_t *data)
{
    uint8_t raw[8];

    if ((sensor == NULL) || (data == NULL))
    {
        return false;
    }

    if (!COMPASS_ReadRegisters(sensor,
                               COMPASS_REG_XOUT0,
                               raw,
                               sizeof(raw)))
    {
        return false;
    }

    COMPASS_DecodeMeasurement(raw, sensor, data);

    return true;
}

bool COMPASS_Measure(compass_t *sensor, compass_data_t *data)
{
    if ((sensor == NULL) || (data == NULL))
    {
        return false;
    }

    /*
     * Trigger one magnetic-field measurement.
     */
    if (!COMPASS_WriteRegister(sensor,
                               COMPASS_REG_CONTROL0,
                               COMPASS_CONTROL0_TM_M))
    {
        return false;
    }

    /*
     * Poll the Meas_M_Done status bit instead of relying on a fixed delay.
     */
    if (!COMPASS_WaitForMagneticMeasurement(
            sensor,
            COMPASS_MEASUREMENT_TIMEOUT_US))
    {
        return false;
    }

    return COMPASS_ReadData(sensor, data);
}

bool COMPASS_CalibrateBridgeOffset(compass_t *sensor)
{
    compass_data_t set_data;
    compass_data_t reset_data;

    if (sensor == NULL)
    {
        return false;
    }

    /*
     * Temporarily disable offset subtraction while measuring the offset
     * itself.
     */
    COMPASS_ClearBridgeOffset(sensor);

    /*
     * SET measurement:
     *
     *   Output_SET = +H + Offset
     */
    if (!COMPASS_Set(sensor))
    {
        return false;
    }

    if (!COMPASS_Measure(sensor, &set_data))
    {
        return false;
    }

    /*
     * RESET measurement:
     *
     *   Output_RESET = -H + Offset
     */
    if (!COMPASS_Reset(sensor))
    {
        return false;
    }

    if (!COMPASS_Measure(sensor, &reset_data))
    {
        return false;
    }

    /*
     * Bridge/null offset:
     *
     *   Offset = (Output_SET + Output_RESET) / 2
     */
    sensor->bridge_offset_gauss[0] =
        (set_data.x_gauss_raw + reset_data.x_gauss_raw) * 0.5f;

    sensor->bridge_offset_gauss[1] =
        (set_data.y_gauss_raw + reset_data.y_gauss_raw) * 0.5f;

    sensor->bridge_offset_gauss[2] =
        (set_data.z_gauss_raw + reset_data.z_gauss_raw) * 0.5f;

    sensor->bridge_offset_valid = true;

    /*
     * RESET flips the magnetic sensing polarity. Restore SET polarity so
     * later readings are +H + Offset; subtracting the stored Offset then
     * yields +H.
     */
    if (!COMPASS_Set(sensor))
    {
        sensor->bridge_offset_valid = false;
        return false;
    }

    return true;
}

void COMPASS_ClearBridgeOffset(compass_t *sensor)
{
    if (sensor == NULL)
    {
        return;
    }

    sensor->bridge_offset_gauss[0] = 0.0f;
    sensor->bridge_offset_gauss[1] = 0.0f;
    sensor->bridge_offset_gauss[2] = 0.0f;

    sensor->bridge_offset_valid = false;
}

status_t COMPASS_LastI2CStatus(const compass_t *sensor)
{
    if (sensor == NULL)
    {
        return kStatus_Fail;
    }

    return sensor->last_i2c_status;
}
