/*
 * compass.h
 *
 * Pure-C MMC5983MA driver interface for FRDM-KL25Z / MCUXpresso SDK.
 *
 * Converted from the Blue Robotics mmc5983-python driver, with register
 * definitions checked against the MEMSIC MMC5983MA Rev A datasheet.
 *
 * Hardware interface:
 *   FRDM-KL25Z I2C0
 *   PTC8 = I2C0_SCL
 *   PTC9 = I2C0_SDA
 *
 * This driver uses fsl_i2c.h. No Arduino, Python, SMBus, or SPI dependency.
 */

#ifndef COMPASS_H_
#define COMPASS_H_

#include <stdbool.h>
#include <stdint.h>
#include "fsl_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COMPASS_I2C_ADDRESS                 (0x30U)
#define COMPASS_PRODUCT_ID_EXPECTED         (0x30U)

#define COMPASS_RAW_ZERO                    (0x20000L)
#define COMPASS_COUNTS_PER_GAUSS            (16384.0f)

#define COMPASS_REG_XOUT0                   (0x00U)
#define COMPASS_REG_XOUT1                   (0x01U)
#define COMPASS_REG_YOUT0                   (0x02U)
#define COMPASS_REG_YOUT1                   (0x03U)
#define COMPASS_REG_ZOUT0                   (0x04U)
#define COMPASS_REG_ZOUT1                   (0x05U)
#define COMPASS_REG_XYZOUT2                 (0x06U)
#define COMPASS_REG_TOUT                    (0x07U)
#define COMPASS_REG_STATUS                  (0x08U)
#define COMPASS_REG_CONTROL0                (0x09U)
#define COMPASS_REG_CONTROL1                (0x0AU)
#define COMPASS_REG_CONTROL2                (0x0BU)
#define COMPASS_REG_CONTROL3                (0x0CU)
#define COMPASS_REG_PRODUCT_ID              (0x2FU)

#define COMPASS_STATUS_MEAS_M_DONE          (1U << 0)
#define COMPASS_STATUS_MEAS_T_DONE          (1U << 1)
#define COMPASS_STATUS_OTP_READ_DONE        (1U << 4)

#define COMPASS_CONTROL0_TM_M               (1U << 0)
#define COMPASS_CONTROL0_TM_T               (1U << 1)
#define COMPASS_CONTROL0_INT_MEAS_DONE_EN   (1U << 2)
#define COMPASS_CONTROL0_SET                (1U << 3)
#define COMPASS_CONTROL0_RESET              (1U << 4)
#define COMPASS_CONTROL0_AUTO_SR_EN         (1U << 5)
#define COMPASS_CONTROL0_OTP_READ           (1U << 6)

#define COMPASS_CONTROL1_BW_MASK            (0x03U)
#define COMPASS_CONTROL1_X_INHIBIT          (1U << 2)
#define COMPASS_CONTROL1_Y_INHIBIT          (1U << 3)
#define COMPASS_CONTROL1_Z_INHIBIT          (1U << 4)
#define COMPASS_CONTROL1_SW_RESET           (1U << 7)

#define COMPASS_CONTROL2_CM_FREQ_MASK       (0x07U)
#define COMPASS_CONTROL2_CMM_EN             (1U << 3)
#define COMPASS_CONTROL2_PRD_SET_MASK       (0x70U)
#define COMPASS_CONTROL2_EN_PRD_SET         (1U << 7)

#define COMPASS_CM_FREQ_OFF                 (0x00U)
#define COMPASS_CM_FREQ_1_HZ                (0x01U)
#define COMPASS_CM_FREQ_10_HZ               (0x02U)
#define COMPASS_CM_FREQ_20_HZ               (0x03U)
#define COMPASS_CM_FREQ_50_HZ               (0x04U)
#define COMPASS_CM_FREQ_100_HZ              (0x05U)
#define COMPASS_CM_FREQ_200_HZ              (0x06U)
#define COMPASS_CM_FREQ_1000_HZ             (0x07U)

#define COMPASS_SOFTWARE_RESET_DELAY_US     (15000U)
#define COMPASS_SET_RESET_DELAY_US          (1000U)
#define COMPASS_MEASUREMENT_TIMEOUT_US      (20000U)
#define COMPASS_STATUS_POLL_INTERVAL_US     (100U)

typedef enum
{
    kCompassBandwidth100Hz = 0U,
    kCompassBandwidth200Hz = 1U,
    kCompassBandwidth400Hz = 2U,
    kCompassBandwidth800Hz = 3U
} compass_bandwidth_t;

typedef struct
{
    int32_t x_raw;
    int32_t y_raw;
    int32_t z_raw;

    float x_gauss_raw;
    float y_gauss_raw;
    float z_gauss_raw;

    float x_gauss;
    float y_gauss;
    float z_gauss;

    uint8_t temperature_raw;
    float temperature_c;
} compass_data_t;

typedef struct
{
    I2C_Type *i2c_base;
    status_t last_i2c_status;

    float bridge_offset_gauss[3];

    uint8_t product_id;
    bool bridge_offset_valid;
} compass_t;

void COMPASS_Init(compass_t *sensor, I2C_Type *i2c_base);
bool COMPASS_Begin(compass_t *sensor);
bool COMPASS_ReadProductId(compass_t *sensor, uint8_t *product_id);
bool COMPASS_SoftwareReset(compass_t *sensor);
bool COMPASS_SetBandwidth(compass_t *sensor, compass_bandwidth_t bandwidth);

bool COMPASS_Set(compass_t *sensor);
bool COMPASS_Reset(compass_t *sensor);

bool COMPASS_CalibrateBridgeOffset(compass_t *sensor);

bool COMPASS_Measure(compass_t *sensor, compass_data_t *data);
bool COMPASS_ReadData(compass_t *sensor, compass_data_t *data);

bool COMPASS_WaitForMagneticMeasurement(compass_t *sensor,
                                        uint32_t timeout_us);

void COMPASS_ClearBridgeOffset(compass_t *sensor);
status_t COMPASS_LastI2CStatus(const compass_t *sensor);

#ifdef __cplusplus
}
#endif

#endif /* COMPASS_H_ */
