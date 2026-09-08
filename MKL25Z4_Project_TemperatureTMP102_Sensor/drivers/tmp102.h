#ifndef TMP102_H
#define TMP102_H

#include "fsl_i2c.h"
#include <stdbool.h>
#include <stdint.h>

// TMP102 I2C default address
#define TMP102_DEFAULT_ADDRESS 0x48

#ifdef __cplusplus
extern "C" {
#endif

// Initialize TMP102 sensor with I2C base and address
void TMP102_Init(I2C_Type *base, uint8_t address);

// Read temperature in Celsius
float TMP102_ReadTempC(void);

// Read temperature in Fahrenheit
float TMP102_ReadTempF(void);

// Check if alert is active
bool TMP102_IsAlertActive(void);

// Set conversion rate (0–3): 0=0.25Hz, 1=1Hz, 2=4Hz, 3=8Hz
void TMP102_SetConversionRate(uint8_t rate);

// Enable or disable extended mode
void TMP102_SetExtendedMode(bool enable);

#ifdef __cplusplus
}
#endif

#endif
