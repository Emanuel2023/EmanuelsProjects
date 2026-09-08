#include "sfe_lsm6dsv16x.h"
#include <string.h>
#include "fsl_common.h"





static void LSM6DSV16X_DelayMs(volatile uint32_t ms)
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



/*
 * KL25Z / MCUXpresso C port of the SparkFun LSM6DSV16X device layer.
 *
 * The original C++ QwDevLSM6DSV16X class has been replaced by the
 * sfe_lsm6dsv16x_t device structure declared in sfe_lsm6dsv16x.h.
 * All sensor register operations still use STMicroelectronics' native
 * lsm6dsv16x_reg.c API through dev->dev_ctx.
 */

bool SFE_LSM6DSV16X_Init(sfe_lsm6dsv16x_t *dev,
                         void *i2c_handle,
                         uint8_t address)
{
    if ((dev == NULL) || (i2c_handle == NULL))
    {
        return false;
    }

    if ((address != LSM6DSV16X_ADDRESS_LOW) &&
        (address != LSM6DSV16X_ADDRESS_HIGH))
    {
        return false;
    }

    memset(dev, 0, sizeof(*dev));

    dev->platform.i2c_base = i2c_handle;
    dev->platform.i2c_address = address;

    LSM6DSV16X_InitContext(&dev->platform, &dev->dev_ctx);

    return SFE_LSM6DSV16X_IsConnected(dev);
}

bool SFE_LSM6DSV16X_IsConnected(sfe_lsm6dsv16x_t *dev)
{
    if (dev == NULL)
    {
        return false;
    }

    return (SFE_LSM6DSV16X_GetUniqueId(dev) == LSM6DSV16X_ID);
}

int32_t SFE_LSM6DSV16X_WriteRegisterRegion(sfe_lsm6dsv16x_t *dev,
                                           uint8_t reg,
                                           uint8_t *data,
                                           uint16_t length)
{
    if (dev == NULL)
    {
        return -1;
    }

    return LSM6DSV16X_PlatformWrite(&dev->platform, reg, data, length);
}

int32_t SFE_LSM6DSV16X_ReadRegisterRegion(sfe_lsm6dsv16x_t *dev,
                                          uint8_t reg,
                                          uint8_t *data,
                                          uint16_t length)
{
    if (dev == NULL)
    {
        return -1;
    }

    return LSM6DSV16X_PlatformRead(&dev->platform, reg, data, length);
}

bool SFE_LSM6DSV16X_SetAccelFullScale(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_xl_full_scale_t scale)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal = lsm6dsv16x_xl_full_scale_set(&dev->dev_ctx, scale);

    dev->fullScaleAccel = scale;
    dev->accelScaleSet = true;

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetGyroFullScale(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_gy_full_scale_t scale)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal = lsm6dsv16x_gy_full_scale_set(&dev->dev_ctx, scale);

    dev->fullScaleGyro = scale;
    dev->gyroScaleSet = true;

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_GetAccelFullScale(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_xl_full_scale_t *scale)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal = lsm6dsv16x_xl_full_scale_get(&dev->dev_ctx, scale);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_GetGyroFullScale(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_gy_full_scale_t *scale)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal = lsm6dsv16x_gy_full_scale_get(&dev->dev_ctx, scale);

    if (retVal != 0)
        return false;

    return true;

}

uint8_t SFE_LSM6DSV16X_GetUniqueId(sfe_lsm6dsv16x_t *dev)
{
    if (dev == NULL)
    {
        return 0;
    }


    uint8_t buff = 0;
    int32_t retVal = (lsm6dsv16x_device_id_get(&dev->dev_ctx, &buff));

    if (retVal != 0)
        return 0;

    return buff;

}

bool SFE_LSM6DSV16X_GetRawTemp(sfe_lsm6dsv16x_t *dev, int16_t *tempVal)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal = lsm6dsv16x_temperature_raw_get(&dev->dev_ctx, tempVal);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_GetRawAccel(sfe_lsm6dsv16x_t *dev, sfe_lsm_raw_data_t *accelData)
{
    if (dev == NULL)
    {
        return false;
    }

    int16_t tempVal[3] = {0};
    int32_t retVal = lsm6dsv16x_acceleration_raw_get(&dev->dev_ctx, tempVal);

    if (retVal != 0)
        return false;

    accelData->xData = tempVal[0];
    accelData->yData = tempVal[1];
    accelData->zData = tempVal[2];

    return true;

}

bool SFE_LSM6DSV16X_GetRawGyro(sfe_lsm6dsv16x_t *dev, sfe_lsm_raw_data_t *gyroData)
{
    if (dev == NULL)
    {
        return false;
    }


    int16_t tempVal[3] = {0};
    int32_t retVal = lsm6dsv16x_angular_rate_raw_get(&dev->dev_ctx, tempVal);

    if (retVal != 0)
        return false;

    gyroData->xData = tempVal[0];
    gyroData->yData = tempVal[1];
    gyroData->zData = tempVal[2];

    return true;

}

bool SFE_LSM6DSV16X_GetRawQvar(sfe_lsm6dsv16x_t *dev, int16_t *qvarData)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal = lsm6dsv16x_ah_qvar_raw_get(&dev->dev_ctx, qvarData);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_GetAccel(sfe_lsm6dsv16x_t *dev, sfe_lsm_data_t *accelData)
{
    if (dev == NULL)
    {
        return false;
    }


    int16_t tempVal[3] = {0};
    int32_t retVal;

    retVal = lsm6dsv16x_acceleration_raw_get(&dev->dev_ctx, tempVal);

    if (dev->accelScaleSet == false)
    {
        SFE_LSM6DSV16X_GetAccelFullScale(dev, &dev->fullScaleAccel);
        dev->accelScaleSet = true;
    }

    if (retVal != 0)
        return false;

    // "fullAcaleAccel" is a private variable that keeps track of the users settings
    // so that the register values can be converted accordingly
    switch (dev->fullScaleAccel)
    {
    case LSM6DSV16X_2g:
        accelData->xData = SFE_LSM6DSV16X_Convert2gToMg(tempVal[0]);
        accelData->yData = SFE_LSM6DSV16X_Convert2gToMg(tempVal[1]);
        accelData->zData = SFE_LSM6DSV16X_Convert2gToMg(tempVal[2]);
        break;
    case LSM6DSV16X_4g:
        accelData->xData = SFE_LSM6DSV16X_Convert4gToMg(tempVal[0]);
        accelData->yData = SFE_LSM6DSV16X_Convert4gToMg(tempVal[1]);
        accelData->zData = SFE_LSM6DSV16X_Convert4gToMg(tempVal[2]);
        break;
    case LSM6DSV16X_8g:
        accelData->xData = SFE_LSM6DSV16X_Convert8gToMg(tempVal[0]);
        accelData->yData = SFE_LSM6DSV16X_Convert8gToMg(tempVal[1]);
        accelData->zData = SFE_LSM6DSV16X_Convert8gToMg(tempVal[2]);
        break;
    case LSM6DSV16X_16g:
        accelData->xData = SFE_LSM6DSV16X_Convert16gToMg(tempVal[0]);
        accelData->yData = SFE_LSM6DSV16X_Convert16gToMg(tempVal[1]);
        accelData->zData = SFE_LSM6DSV16X_Convert16gToMg(tempVal[2]);
        break;
    default:
        return false; // Something has gone wrong
    }

    return true;

}

bool SFE_LSM6DSV16X_GetGyro(sfe_lsm6dsv16x_t *dev, sfe_lsm_data_t *gyroData)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;
    int16_t tempVal[3] = {0};

    retVal = lsm6dsv16x_angular_rate_raw_get(&dev->dev_ctx, tempVal);

    if (dev->gyroScaleSet == false)
    {
        SFE_LSM6DSV16X_GetGyroFullScale(dev, &dev->fullScaleGyro);
        dev->gyroScaleSet = true;
    }

    if (retVal != 0)
        return false;

    // "dev->fullScaleGyro" is a private variable that keeps track of the users settings
    // so that the register values can be converted accordingly
    switch (dev->fullScaleGyro)
    {
    case LSM6DSV16X_125dps:
        gyroData->xData = SFE_LSM6DSV16X_Convert125dpsToMdps(tempVal[0]);
        gyroData->yData = SFE_LSM6DSV16X_Convert125dpsToMdps(tempVal[1]);
        gyroData->zData = SFE_LSM6DSV16X_Convert125dpsToMdps(tempVal[2]);
        break;
    case LSM6DSV16X_250dps:
        gyroData->xData = SFE_LSM6DSV16X_Convert250dpsToMdps(tempVal[0]);
        gyroData->yData = SFE_LSM6DSV16X_Convert250dpsToMdps(tempVal[1]);
        gyroData->zData = SFE_LSM6DSV16X_Convert250dpsToMdps(tempVal[2]);
        break;
    case LSM6DSV16X_500dps:
        gyroData->xData = SFE_LSM6DSV16X_Convert500dpsToMdps(tempVal[0]);
        gyroData->yData = SFE_LSM6DSV16X_Convert500dpsToMdps(tempVal[1]);
        gyroData->zData = SFE_LSM6DSV16X_Convert500dpsToMdps(tempVal[2]);
        break;
    case LSM6DSV16X_1000dps:
        gyroData->xData = SFE_LSM6DSV16X_Convert1000dpsToMdps(tempVal[0]);
        gyroData->yData = SFE_LSM6DSV16X_Convert1000dpsToMdps(tempVal[1]);
        gyroData->zData = SFE_LSM6DSV16X_Convert1000dpsToMdps(tempVal[2]);
        break;
    case LSM6DSV16X_2000dps:
        gyroData->xData = SFE_LSM6DSV16X_Convert2000dpsToMdps(tempVal[0]);
        gyroData->yData = SFE_LSM6DSV16X_Convert2000dpsToMdps(tempVal[1]);
        gyroData->zData = SFE_LSM6DSV16X_Convert2000dpsToMdps(tempVal[2]);
        break;
    case LSM6DSV16X_4000dps:
        gyroData->xData = SFE_LSM6DSV16X_Convert4000dpsToMdps(tempVal[0]);
        gyroData->yData = SFE_LSM6DSV16X_Convert4000dpsToMdps(tempVal[1]);
        gyroData->zData = SFE_LSM6DSV16X_Convert4000dpsToMdps(tempVal[2]);
        break;
    default:
        return false; // Something has gone wrong
    }

    return true;

}

float SFE_LSM6DSV16X_Convert2gToMg(int16_t data)
{
    return (lsm6dsv16x_from_fs2_to_mg(data));

}

float SFE_LSM6DSV16X_Convert4gToMg(int16_t data)
{
    return (lsm6dsv16x_from_fs4_to_mg(data));

}

float SFE_LSM6DSV16X_Convert8gToMg(int16_t data)
{
    return (lsm6dsv16x_from_fs8_to_mg(data));

}

float SFE_LSM6DSV16X_Convert16gToMg(int16_t data)
{
    return (lsm6dsv16x_from_fs16_to_mg(data));

}

float SFE_LSM6DSV16X_Convert125dpsToMdps(int16_t data)
{
    return (lsm6dsv16x_from_fs125_to_mdps(data));

}

float SFE_LSM6DSV16X_Convert250dpsToMdps(int16_t data)
{
    return (lsm6dsv16x_from_fs250_to_mdps(data));

}

float SFE_LSM6DSV16X_Convert500dpsToMdps(int16_t data)
{
    return (lsm6dsv16x_from_fs500_to_mdps(data));

}

float SFE_LSM6DSV16X_Convert1000dpsToMdps(int16_t data)
{
    return (lsm6dsv16x_from_fs1000_to_mdps(data));

}

float SFE_LSM6DSV16X_Convert2000dpsToMdps(int16_t data)
{
    return (lsm6dsv16x_from_fs2000_to_mdps(data));

}

float SFE_LSM6DSV16X_Convert4000dpsToMdps(int16_t data)
{
    return (lsm6dsv16x_from_fs4000_to_mdps(data));

}

float SFE_LSM6DSV16X_ConvertToCelsius(int16_t data)
{
    return (lsm6dsv16x_from_lsb_to_celsius(data));

}

bool SFE_LSM6DSV16X_DeviceReset(sfe_lsm6dsv16x_t *dev)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_reset_set(&dev->dev_ctx, (lsm6dsv16x_reset_t)1);

    if (retVal != 0)
    {
        return false;
    }

    dev->accelScaleSet = false;

    return true;

}

bool SFE_LSM6DSV16X_GetDeviceReset(sfe_lsm6dsv16x_t *dev)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;
    lsm6dsv16x_reset_t tempVal;

    retVal = lsm6dsv16x_reset_get(&dev->dev_ctx, &tempVal);

    if (retVal != 0)
        return false;

    if ((tempVal) == 0x00)
        return true;

    return false;

}

bool SFE_LSM6DSV16X_GetAutoIncrement(sfe_lsm6dsv16x_t *dev)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;
    uint8_t tempVal;

    retVal = lsm6dsv16x_auto_increment_get(&dev->dev_ctx, &tempVal);

    if (retVal != 0)
        return false;

    if ((tempVal) == 0x00)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetAccelMode(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_xl_mode_t mode)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_xl_mode_set(&dev->dev_ctx, mode);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetGyroMode(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_gy_mode_t mode)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_gy_mode_set(&dev->dev_ctx, mode);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_EnableAccelHpFilter(sfe_lsm6dsv16x_t *dev, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_filt_xl_hp_set(&dev->dev_ctx, (uint8_t)enable);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_EnableFilterSettling(sfe_lsm6dsv16x_t *dev, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;
    lsm6dsv16x_filt_settling_mask_t sfe_filt_mask;

    sfe_filt_mask.drdy = enable;
    sfe_filt_mask.irq_xl = enable;
    sfe_filt_mask.irq_g = enable;

    retVal = lsm6dsv16x_filt_settling_mask_set(&dev->dev_ctx, sfe_filt_mask);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_EnableAccelLPS2(sfe_lsm6dsv16x_t *dev, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_filt_xl_lp2_set(&dev->dev_ctx, (uint8_t)enable);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_EnableFastSetMode(sfe_lsm6dsv16x_t *dev, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_filt_xl_fast_settling_set(&dev->dev_ctx, (uint8_t)enable);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_EnableGyroLP1Filter(sfe_lsm6dsv16x_t *dev, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_filt_gy_lp1_set(&dev->dev_ctx, (uint8_t)enable);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetGyroLP1Bandwidth(sfe_lsm6dsv16x_t *dev, uint8_t val)
{
    if (dev == NULL)
    {
        return false;
    }

    if (val > 0x07)
        return false;

    int32_t retVal;

    retVal = lsm6dsv16x_filt_gy_lp1_bandwidth_set(&dev->dev_ctx, (lsm6dsv16x_filt_gy_lp1_bandwidth_t)val);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_EnableAccelLP2Filter(sfe_lsm6dsv16x_t *dev, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_filt_xl_lp2_set(&dev->dev_ctx, (uint8_t)enable);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetAccelLP2Bandwidth(sfe_lsm6dsv16x_t *dev, uint8_t val)
{
    if (dev == NULL)
    {
        return false;
    }

    if (val > 0x07)
        return false;

    int32_t retVal;

    retVal = lsm6dsv16x_filt_xl_lp2_set(&dev->dev_ctx, (lsm6dsv16x_filt_xl_lp2_bandwidth_t)val);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_EnableBlockDataUpdate(sfe_lsm6dsv16x_t *dev, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;

    retVal = lsm6dsv16x_block_data_update_set(&dev->dev_ctx, (uint8_t)enable);

    if (retVal != 0)
        return false;

    return true;

}

uint8_t SFE_LSM6DSV16X_GetBlockDataUpdate(sfe_lsm6dsv16x_t *dev)
{
    if (dev == NULL)
    {
        return 0;
    }


    uint8_t tempVal;
    int32_t retVal = 0;

    retVal = lsm6dsv16x_block_data_update_get(&dev->dev_ctx, &tempVal);

    if (retVal != 0)
        return 0x02; // not a bit that can be returned

    return tempVal;

}

bool SFE_LSM6DSV16X_SetAccelDataRate(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_data_rate_t rate)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_xl_data_rate_set(&dev->dev_ctx, rate);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetGyroDataRate(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_data_rate_t rate)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_gy_data_rate_set(&dev->dev_ctx, rate);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_EnableTimestamp(sfe_lsm6dsv16x_t *dev, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_timestamp_set(&dev->dev_ctx, (uint8_t)enable);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetFifoWatermark(sfe_lsm6dsv16x_t *dev, uint8_t val)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_fifo_watermark_set(&dev->dev_ctx, val);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetFifoMode(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_fifo_mode_t mode)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_fifo_mode_set(&dev->dev_ctx, mode);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetAccelFifoBatchSet(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_fifo_xl_batch_t odr)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_fifo_xl_batch_set(&dev->dev_ctx, odr);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetGyroFifoBatchSet(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_fifo_gy_batch_t odr)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_fifo_gy_batch_set(&dev->dev_ctx, odr);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetFifoTimestampDec(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_fifo_timestamp_batch_t decimation)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_fifo_timestamp_batch_set(&dev->dev_ctx, decimation);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_GetAllInterrupts(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_all_sources_t *source)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal = 0;

    // 0 = active high :  active low, 1
    retVal = lsm6dsv16x_all_sources_get(&dev->dev_ctx, source);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetInt2DENActiveLow(sfe_lsm6dsv16x_t *dev, bool activeLow)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal = 0;

    // 0 = active high :  active low, 1
    retVal = lsm6dsv16x_den_polarity_set(&dev->dev_ctx, (lsm6dsv16x_den_polarity_t)activeLow);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetIntRoute(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_pin_int_route_t val, sfe_lsm_pin_t pin)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal = 0;

    if (pin == LSM_PIN_ONE)
        retVal = lsm6dsv16x_pin_int1_route_set(&dev->dev_ctx, &val);
    if (pin == LSM_PIN_TWO)
        retVal = lsm6dsv16x_pin_int2_route_set(&dev->dev_ctx, &val);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetIntAccelDataReady(sfe_lsm6dsv16x_t *dev, sfe_lsm_pin_t pin, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }


    lsm6dsv16x_pin_int_route_t int_route = {0};
    int_route.drdy_xl = (uint8_t)enable;

    if (SFE_LSM6DSV16X_SetIntRoute(dev, int_route, pin))
        return true;

    return false;

}

bool SFE_LSM6DSV16X_SetIntGyroDataReady(sfe_lsm6dsv16x_t *dev, sfe_lsm_pin_t pin, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }


    lsm6dsv16x_pin_int_route_t int_route = {0};
    int_route.drdy_g = (uint8_t)enable;

    if (SFE_LSM6DSV16X_SetIntRoute(dev, int_route, pin))
        return true;

    return false;

}

bool SFE_LSM6DSV16X_SetIntSingleTap(sfe_lsm6dsv16x_t *dev, sfe_lsm_pin_t pin, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }


    lsm6dsv16x_pin_int_route_t int_route = {0};
    int_route.single_tap = (uint8_t)enable;

    if (SFE_LSM6DSV16X_SetIntRoute(dev, int_route, pin))
        return true;

    return false;

}

bool SFE_LSM6DSV16X_SetIntDoubleTap(sfe_lsm6dsv16x_t *dev, sfe_lsm_pin_t pin, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }

    lsm6dsv16x_pin_int_route_t int_route = {0};
    int_route.double_tap = (uint8_t)enable;

    if (SFE_LSM6DSV16X_SetIntRoute(dev, int_route, pin))
        return true;

    return false;

}

bool SFE_LSM6DSV16X_SetIntWakeup(sfe_lsm6dsv16x_t *dev, sfe_lsm_pin_t pin, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }


    lsm6dsv16x_pin_int_route_t int_route = {0};
    int_route.wakeup = (uint8_t)enable;

    if (SFE_LSM6DSV16X_SetIntRoute(dev, int_route, pin))
        return true;

    return false;

}

bool SFE_LSM6DSV16X_SetIntFreeFall(sfe_lsm6dsv16x_t *dev, sfe_lsm_pin_t pin, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }


    lsm6dsv16x_pin_int_route_t int_route = {0};
    int_route.freefall = (uint8_t)enable;

    if (SFE_LSM6DSV16X_SetIntRoute(dev, int_route, pin))
        return true;

    return false;

}

bool SFE_LSM6DSV16X_SetIntSleepChange(sfe_lsm6dsv16x_t *dev, sfe_lsm_pin_t pin, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }


    lsm6dsv16x_pin_int_route_t int_route = {0};
    int_route.sleep_change = (uint8_t)enable;

    if (SFE_LSM6DSV16X_SetIntRoute(dev, int_route, pin))
        return true;

    return false;

}

bool SFE_LSM6DSV16X_SetDataReadyMode(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_data_ready_mode_t pulse)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_data_ready_mode_set(&dev->dev_ctx, pulse);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_EnableTapInterrupt(sfe_lsm6dsv16x_t *dev, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;
    lsm6dsv16x_interrupt_mode_t intEnable = {0};

    intEnable.enable = (uint8_t)enable;

    retVal = lsm6dsv16x_interrupt_enable_set(&dev->dev_ctx, intEnable);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetTapMode(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_tap_mode_t mode)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_tap_mode_set(&dev->dev_ctx, mode);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_GetTapMode(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_tap_mode_t *mode)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;

    retVal = lsm6dsv16x_tap_mode_get(&dev->dev_ctx, mode);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetTapDirection(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_tap_detection_t directionDetect)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;

    retVal = lsm6dsv16x_tap_detection_set(&dev->dev_ctx, directionDetect);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_GetTapDirection(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_tap_detection_t *directionDetect)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;

    retVal = lsm6dsv16x_tap_detection_get(&dev->dev_ctx, directionDetect);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetTapThresholds(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_tap_thresholds_t thresholds)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;

    retVal = lsm6dsv16x_tap_thresholds_set(&dev->dev_ctx, thresholds);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_GetTapThresholds(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_tap_thresholds_t *thresholds)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;

    retVal = lsm6dsv16x_tap_thresholds_get(&dev->dev_ctx, thresholds);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetTapTimeWindows(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_tap_time_windows_t window)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_tap_time_windows_set(&dev->dev_ctx, window);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_GetTapTimeWindows(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_tap_time_windows_t *window)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;

    retVal = lsm6dsv16x_tap_time_windows_get(&dev->dev_ctx, window);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_EnableAhQvar(sfe_lsm6dsv16x_t *dev, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;
    lsm6dsv16x_ah_qvar_mode_t qvar = {0};

    if (enable)
        qvar.ah_qvar_en = 1;
    else
        qvar.ah_qvar_en = 0;

    retVal = lsm6dsv16x_ah_qvar_mode_set(&dev->dev_ctx, qvar);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_GetQvarMode(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_ah_qvar_mode_t *mode)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_ah_qvar_mode_get(&dev->dev_ctx, mode);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetQvarImpedance(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_ah_qvar_zin_t val)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;
    retVal = lsm6dsv16x_ah_qvar_zin_set(&dev->dev_ctx, val);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_CheckQvar(sfe_lsm6dsv16x_t *dev)
{
    if (dev == NULL)
    {
        return false;
    }

    lsm6dsv16x_all_sources_t tempVal;

    int32_t retVal = lsm6dsv16x_all_sources_get(&dev->dev_ctx, &tempVal);

    if (retVal != 0)
        return false;

    if (tempVal.drdy_ah_qvar == 1)
        return true;

    return false;

}

bool SFE_LSM6DSV16X_SetHubODR(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_sh_data_rate_t rate)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal = lsm6dsv16x_sh_data_rate_set(&dev->dev_ctx, rate);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetHubSensorRead(sfe_lsm6dsv16x_t *dev, uint8_t sensor, lsm6dsv16x_sh_cfg_read_t *settings)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    if (sensor > 3)
        return false;

    switch (sensor)
    {
    case 0:
        retVal = lsm6dsv16x_sh_slv0_cfg_read(&dev->dev_ctx, settings);
        break;
    case 1:
        retVal = lsm6dsv16x_sh_slv1_cfg_read(&dev->dev_ctx, settings);
        break;
    case 2:
        retVal = lsm6dsv16x_sh_slv2_cfg_read(&dev->dev_ctx, settings);
        break;
    case 3:
        retVal = lsm6dsv16x_sh_slv3_cfg_read(&dev->dev_ctx, settings);
        break;
    default:
        return false;
    }

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetHubSensorWrite(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_sh_cfg_write_t *settings)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_sh_cfg_write(&dev->dev_ctx, settings);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetNumberHubSensors(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_sh_slave_connected_t numSensors)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;

    retVal = lsm6dsv16x_sh_slave_connected_set(&dev->dev_ctx, numSensors);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_EnableAuxiliaryI2C(sfe_lsm6dsv16x_t *dev, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_sh_master_set(&dev->dev_ctx, (uint8_t)enable);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_ReadPeripheralSensor(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_emb_sh_read_t *shReg, uint8_t len)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_sh_read_data_raw_get(&dev->dev_ctx, shReg, len);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_GetExternalSensorNack(sfe_lsm6dsv16x_t *dev, uint8_t sensor)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;
    lsm6dsv16x_all_sources_t tempVal;

    retVal = lsm6dsv16x_all_sources_get(&dev->dev_ctx, &tempVal);

    if (retVal != 0)
        return false;

    switch (sensor)
    {
    case 0:
        if (tempVal.sh_slave0_nack == 1)
            return true;
        break;
    case 1:
        if (tempVal.sh_slave1_nack == 1)
            return true;
        break;
    case 2:
        if (tempVal.sh_slave2_nack == 1)
            return true;
        break;
    case 3:
        if (tempVal.sh_slave3_nack == 1)
            return true;
        break;
    default:
        return false;
    }

    return false;

}

bool SFE_LSM6DSV16X_EnableHubWriteOnceMode(sfe_lsm6dsv16x_t *dev, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    // 0 = Write each cycle
    // 1 = Write once
    retVal = lsm6dsv16x_sh_write_mode_set(&dev->dev_ctx, (lsm6dsv16x_sh_write_mode_t)enable);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_EnableHubPassThrough(sfe_lsm6dsv16x_t *dev, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal = lsm6dsv16x_sh_pass_through_set(&dev->dev_ctx, (uint8_t)enable);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_EnableHubPullUps(sfe_lsm6dsv16x_t *dev, bool enable)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;

    if (enable)
        retVal = lsm6dsv16x_sh_master_interface_pull_up_set(&dev->dev_ctx, 1);
    else
        retVal = lsm6dsv16x_sh_master_interface_pull_up_set(&dev->dev_ctx, 0);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_ResetSensorHub(sfe_lsm6dsv16x_t *dev)
{
    if (dev == NULL)
    {
        return false;
    }


    int32_t retVal;

    // Must be set to one, then zero
    retVal = lsm6dsv16x_sh_reset_set(&dev->dev_ctx, 1);

    SDK_DelayAtLeastUs(1000U, SystemCoreClock);

    retVal = lsm6dsv16x_sh_reset_set(&dev->dev_ctx, 0);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetAccelSelfTest(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_xl_self_test_t val)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_xl_self_test_set(&dev->dev_ctx, val);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_SetGyroSelfTest(sfe_lsm6dsv16x_t *dev, lsm6dsv16x_gy_self_test_t val)
{
    if (dev == NULL)
    {
        return false;
    }

    int32_t retVal;

    retVal = lsm6dsv16x_gy_self_test_set(&dev->dev_ctx, val);

    if (retVal != 0)
        return false;

    return true;

}

bool SFE_LSM6DSV16X_CheckStatus(sfe_lsm6dsv16x_t *dev)
{
    if (dev == NULL)
    {
        return false;
    }

    lsm6dsv16x_data_ready_t tempVal;
    int32_t retVal = lsm6dsv16x_flag_data_ready_get(&dev->dev_ctx, &tempVal);

    if (retVal != 0)
        return false;

    if ((tempVal.drdy_xl == 1) && (tempVal.drdy_gy == 1))
        return true;

    return false;

}

bool SFE_LSM6DSV16X_CheckAccelStatus(sfe_lsm6dsv16x_t *dev)
{
    if (dev == NULL)
    {
        return false;
    }

    lsm6dsv16x_data_ready_t tempVal;

    int32_t retVal = lsm6dsv16x_flag_data_ready_get(&dev->dev_ctx, &tempVal);

    if (retVal != 0)
        return false;

    if (tempVal.drdy_xl == 1)
        return true;

    return false;

}

bool SFE_LSM6DSV16X_CheckGyroStatus(sfe_lsm6dsv16x_t *dev)
{
    if (dev == NULL)
    {
        return false;
    }

    lsm6dsv16x_data_ready_t tempVal;

    int32_t retVal = lsm6dsv16x_flag_data_ready_get(&dev->dev_ctx, &tempVal);

    if (retVal != 0)
        return false;

    if (tempVal.drdy_gy == 1)
        return true;

    return false;

}

bool SFE_LSM6DSV16X_CheckTempStatus(sfe_lsm6dsv16x_t *dev)
{
    if (dev == NULL)
    {
        return false;
    }

    lsm6dsv16x_data_ready_t tempVal;

    int32_t retVal = lsm6dsv16x_flag_data_ready_get(&dev->dev_ctx, &tempVal);

    if (retVal != 0)
        return false;

    if (tempVal.drdy_temp == 1)
        return true;

    return false;

}
