#ifndef SFE_LSM_SHIM_H_
#define SFE_LSM_SHIM_H_

#include <stdint.h>
#include "lsm6dsv16x_reg.h"

/*
 * KL25Z / MCUXpresso interface for the ST LSM6DSV16X driver.
 */


typedef struct
{
    void *i2c_base;
    uint8_t i2c_address;

} sfe_lsm_platform_t;






int32_t LSM6DSV16X_PlatformWrite(void *handle,
                                 uint8_t reg,
                                 const uint8_t *buf,
                                 uint16_t len);

int32_t LSM6DSV16X_PlatformRead(void *handle,
                                uint8_t reg,
                                uint8_t *buf,
                                uint16_t len);

void LSM6DSV16X_InitContext(void *handle,
                            stmdev_ctx_t *ctx);

#endif /* SFE_LSM_SHIM_H_ */
