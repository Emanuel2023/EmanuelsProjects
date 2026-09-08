#include "sfe_lsm_shim.h"
#include "fsl_i2c.h"
#include "fsl_debug_console.h"
#include <stddef.h>

/*
 * Initializes the STMicroelectronics driver context.
 *
 * For our KL25Z implementation, "handle" is expected to point to the
 * I2C peripheral being used (for example I2C0).
 */
void LSM6DSV16X_InitContext(void *handle, stmdev_ctx_t *dev)
{
    if (dev == NULL)
    {
        return;
    }

    dev->handle    = handle;
    dev->write_reg = LSM6DSV16X_PlatformWrite;
    dev->read_reg  = LSM6DSV16X_PlatformRead;

    /* Delay callback is optional in the ST driver. */
    dev->mdelay = NULL;
}


/*
 * Write one or more bytes to an LSM6DSV16X register using
 * the KL25Z MCUXpresso I2C driver.
 */
int32_t LSM6DSV16X_PlatformWrite(void *handle,
                                 uint8_t reg,
                                 const uint8_t *buf,
                                 uint16_t len)
{
    sfe_lsm_platform_t *platform;
    I2C_Type *i2cBase;
    i2c_master_transfer_t transfer;
    status_t status;

    if ((handle == NULL) || ((buf == NULL) && (len > 0U)))
    {
        return -1;
    }

    platform = (sfe_lsm_platform_t *)handle;

    i2cBase = (I2C_Type *)platform->i2c_base;

    transfer.slaveAddress   = platform->i2c_address;
    transfer.direction      = kI2C_Write;
    transfer.subaddress     = reg;
    transfer.subaddressSize = 1U;
    transfer.data           = (uint8_t *)buf;
    transfer.dataSize       = len;
    transfer.flags          = kI2C_TransferDefaultFlag;

    PRINTF("PlatformWrite: addr=0x%02X reg=0x%02X len=%u\r\n",
           platform->i2c_address,
           reg,
           (unsigned int)len);

    status = I2C_MasterTransferBlocking(i2cBase, &transfer);

    PRINTF("PlatformWrite returned status=%d\r\n",
           (int)status);

    if (status == kStatus_Success)
    {
        return 0;
    }

    return -1;
}


/*
 * Read one or more bytes from an LSM6DSV16X register using
 * the KL25Z MCUXpresso I2C driver.
 */
int32_t LSM6DSV16X_PlatformRead(void *handle,
                                uint8_t reg,
                                uint8_t *buf,
                                uint16_t len)
{
    sfe_lsm_platform_t *platform;
    I2C_Type *i2cBase;
    i2c_master_transfer_t transfer;
    status_t status;

    if ((handle == NULL) || ((buf == NULL) && (len > 0U)))
    {
        return -1;
    }

    /*
     * handle points to our platform structure,
     * NOT directly to I2C0.
     */
    platform = (sfe_lsm_platform_t *)handle;

    /*
     * Extract the actual KL25Z I2C peripheral.
     */
    i2cBase = (I2C_Type *)platform->i2c_base;

    transfer.slaveAddress   = platform->i2c_address;
    transfer.direction      = kI2C_Read;
    transfer.subaddress     = reg;
    transfer.subaddressSize = 1U;
    transfer.data           = buf;
    transfer.dataSize       = len;
    transfer.flags          = kI2C_TransferDefaultFlag;

    PRINTF("PlatformRead: addr=0x%02X reg=0x%02X len=%u\r\n",
           platform->i2c_address,
           reg,
           (unsigned int)len);

    status = I2C_MasterTransferBlocking(i2cBase, &transfer);

    PRINTF("PlatformRead returned status=%d\r\n",
           (int)status);

    if (status == kStatus_Success)
    {
        return 0;
    }

    return -1;
}
